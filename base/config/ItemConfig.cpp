#include "ItemConfig.h"
#include "TableManager.h"
#include "RoleDataConfig.h"
#include "ComposeConfig.h"
#include "xPos.h"
#include "MiscConfig.h"
#include "NpcConfig.h"
#include "FoodConfig.h"
#include "SceneItem.pb.h"
#include "ManualConfig.h"
#include "BaseConfig.h"

void SSuitCFG::getBuffs(DWORD count, TVecDWORD& vecIDs) const
{
  for (auto &v : vecBuffs)
  {
    if (v.first == count)
      vecIDs.insert(vecIDs.end(), v.second.begin(), v.second.end());
  }
}

void SSuitCFG::getRefineBuffs(TVecDWORD& buffs) const
{
  for (auto &v : vecRefineBuffs) {
    buffs.push_back(v);
  }
}

void SSuitCFG::getRefineEquipBuffs(DWORD id, TVecDWORD& buffs) const
{
  auto e = mapEquipRefineBuffs.find(id);
  if (e == mapEquipRefineBuffs.end()) {
    return;
  }
  for (auto &v : e->second.vecBuffs) {
    buffs.push_back(v);
  }
}

void SSuitCFG::getRefineAllBuffs(TVecDWORD& buffs) const
{
  for (auto &v : vecRefineBuffs)
    buffs.push_back(v);
  for (auto it : mapEquipRefineBuffs)
    for (auto &v : it.second.vecBuffs)
      buffs.push_back(v);
}

DWORD SSuitCFG::getEquipRefineLv(DWORD equipid) const
{
  auto v = mapEquipRefineBuffs.find(equipid);
  if (v != mapEquipRefineBuffs.end()) {
    return v->second.dwLevel;
  }
  return DWORD_MAX;
}

// enchant config
float SEnchantAttr::random() const
{
  if (vecItems.empty() == true)
    return randFBetween(fMin, fMax);

  DWORD dwRand = randBetween(0, dwMaxWeight);
  for (auto &v : vecItems)
  {
    if (dwRand <= v.dwWeight)
      return randFBetween(v.fMin, v.fMax);
  }

  return 0.0f;
}
float SEnchantAttr::getRandomRate() const
{
  if (vecItems.size() < 2 || dwMaxWeight == 0)
    return 0.0f;
  
  DWORD count = 0;
  float weight = 0;
  float fAllWeight = dwMaxWeight;
  //后两项的权重和
  for (auto it = vecItems.rbegin(); it != vecItems.rend(); ++it)
  {
    weight += it->dwRawWeight;
    count++;
    if (count >= 2)
      break;
  }  
  return weight / fAllWeight;
}

DWORD SEnchantAttr::randomExtraBuff() const
{
  DWORD dwRand = randBetween(1, dwExtraMaxWeigth);  //[1, dwExtraMaxWeigth]
  for (auto &v : vecExtraBuff)
  {
    if (dwRand <= v.second)
      return v.first;
  }
  XERR << "[附魔] 出错" << dwRand <<"max"<< dwExtraMaxWeigth << XEND;
  return 0;
}

bool SEnchantAttr::canGetExtraBuff(EItemType eType) const
{
  return setNoBuffid.find(eType) == setNoBuffid.end();
}

bool SEnchantAttr::checkBuffId(DWORD buffId) const
{
  for (auto &v : vecExtraBuff)
  {
    if (v.first == buffId)
      return true;
  }
  return false;
}

bool SEnchantEquip::random(DWORD dwNum, vector<EAttrType>& vecAttrs) const
{
  DWORD weight = dwMaxWeight;
  for (DWORD i = 0; i < dwNum; ++i)
  {
    DWORD dwRand = randBetween(1, weight);    //[1, weight] 最小从1开始
    DWORD dwOffset = 0;
    for (auto it = vecItems.begin(); it != vecItems.end(); ++it)
    {
      //概率为0 跳过
      if (it->p.second == 0)
        continue;

      //已随机到的跳过
      auto o = find(vecAttrs.begin(), vecAttrs.end(), it->p.first);
      if (o != vecAttrs.end())
        continue;

      if (dwRand <= (dwOffset + it->p.second))
      {
        vecAttrs.push_back(it->p.first);
        weight -= it->p.second;
        break;
      }
      dwOffset += it->p.second;
    }
  }
  return true;
}

const SEnchantEquipItem* SEnchantEquip::getEnchantEquipItem(EAttrType type) const
{
  auto v = find_if(vecItems.begin(), vecItems.end(), [type](const SEnchantEquipItem& r) -> bool {
    return r.p.first == type;
  });
  if (v != vecItems.end())
    return &(*v);

  return nullptr;
}

const SEnchantAttr* SEnchantCFG::getEnchantAttr(EAttrType eType, EItemType eItem) const
{
  auto v = find_if(vecAttrs.begin(), vecAttrs.end(), [eType, eItem](const SEnchantAttr& r) -> bool{
    return r.eType == eType && r.setValidItemType.find(eItem) != r.setValidItemType.end();
  });
  if (v != vecAttrs.end())
    return &(*v);

  return nullptr;
}

const SEnchantAttr* SEnchantCFG::getEnchantAttr(DWORD dwConfigID) const
{
  auto v = find_if(vecAttrs.begin(), vecAttrs.end(), [dwConfigID](const SEnchantAttr& r) -> bool{
    return r.dwConfigID == dwConfigID;
  });
  if (v != vecAttrs.end())
    return &(*v);

  return nullptr;
}

const SEnchantEquip* SEnchantCFG::getEnchantEquip(EItemType eType) const
{
  auto v = find_if(vecEquips.begin(), vecEquips.end(), [eType](const SEnchantEquip& r) -> bool{
    return r.eItemType == eType;
  });
  if (v != vecEquips.end())
    return &(*v);

  return nullptr;
}

bool SEnchantCFG::random(EItemType eItemType, EnchantData& rData) const
{
  const SEnchantEquip* pEquip = getEnchantEquip(eItemType);
  if (pEquip == nullptr)
    return false;

  vector<EAttrType> vecAttrs;
  if (pEquip->random(dwMaxNum, vecAttrs) == false)
    return false;

  rData.clear_extras();
  vector<EnchantAttr> vecEnchant;
  bool bGot = false;
  for (auto &v : vecAttrs)
  {
    const SEnchantAttr* pAttr = getEnchantAttr(v, eItemType);
    if (pAttr == nullptr)
    {
      XERR << "附魔随机属性 : " << v << " 不合法" << XEND;
      continue;
    }

    const RoleData* pData = RoleDataConfig::getMe().getRoleData(v);
    if (pData == nullptr)
    {
      XERR << "附魔随机属性 : " << v << " 不合法" << XEND;
      continue;
    }

    if (!bGot && contain(vecAttrs, pAttr->vecPairType, v) == true)
    {
      if (pAttr->canGetExtraBuff(eItemType)) 
      {
        EnchantExtra* pExtra = rData.add_extras();
        pExtra->set_buffid(pAttr->randomExtraBuff());
        pExtra->set_configid(pAttr->dwConfigID);
        bGot = true;
      }
      else
      {
        XLOG << "[附魔] itemtype"<<eItemType <<"不可有额外buffid, configid"<<pAttr->dwConfigID << XEND;
      }
    }

    EnchantAttr oAttr;
    oAttr.set_type(v);
    float fValue = pAttr->random();
    fValue = pData->bPercent ? fValue * FLOAT_TO_DWORD : fValue;
    oAttr.set_value(static_cast<DWORD>(fValue));
    vecEnchant.push_back(oAttr);
  }

  rData.set_type(eType);
  rData.clear_attrs();
  for (auto &v : vecEnchant)
    rData.add_attrs()->CopyFrom(v);

  return true;
}

bool SEnchantCFG::specialRandom(EItemType eItemType, EnchantData& rData) const
{
  const SEnchantEquip* pEquip = getEnchantEquip(eItemType);
  if (pEquip == nullptr)
    return false;
  if (rData.attrs_size() <= 1)
    return false;

  if (rData.extras_size() != 0)
    return false;

  vector<EAttrType> vecAttrs;
  vecAttrs.reserve(rData.attrs_size());  
  for (int i = 0; i < rData.attrs_size(); ++i)
  {
    const EnchantAttr& rAttr = rData.attrs(i);
    vecAttrs.push_back(rAttr.type());
  }
  
  static set<EAttrType> s_set1 = { EATTRTYPE_MAXSPPER , EATTRTYPE_MATK };
  static set<EAttrType> s_set2 = { EATTRTYPE_EQUIPASPD, EATTRTYPE_DAMINCREASE };
  
  auto needProcess = [&](EAttrType type, vector<EAttrType>& vecAttrs, set<EAttrType>& s) {
    auto it = s.find(type);
    if (it == s.end())
      return false;
    for (auto& v: vecAttrs)
    {
      if (v == type)
        continue;      
      auto it = s.find(v);
      if (it != s.end())
        return true;
    }
    return false;
  };

  for (auto &v : vecAttrs)
  {
    if (!needProcess(v, vecAttrs, s_set1))
      if (!needProcess(v, vecAttrs, s_set2))
        continue;   

    const SEnchantAttr* pAttr = getEnchantAttr(v, eItemType);
    if (pAttr == nullptr)
    {
      XERR << "[附魔-特殊附魔]" << v << " 不合法" << XEND;
      continue;
    }

    const RoleData* pData = RoleDataConfig::getMe().getRoleData(v);
    if (pData == nullptr)
    {
      XERR << "[附魔-特殊附魔]" << v << " 不合法" << XEND;
      continue;
    }

    if (contain(vecAttrs, pAttr->vecPairType, v) == true)
    {
      if (pAttr->canGetExtraBuff(eItemType))
      {
        EnchantExtra* pExtra = rData.add_extras();
        if (pExtra)
        {
          pExtra->set_buffid(pAttr->randomExtraBuff());
          pExtra->set_configid(pAttr->dwConfigID);
          XLOG << "[附魔-特殊附魔] 遗留属性附魔成功 configid" << pAttr->dwConfigID << "buffid" << pExtra->buffid() << "attrtype" << v << XEND;
          return true;
        }
      }
      else
      {
        XLOG << "[附魔-特殊附魔] itemtype" << eItemType << "不可有额外buffid, configid" << pAttr->dwConfigID << XEND;
      }
    }
  }
  return false;
}

bool SEnchantCFG::contain(const vector<EAttrType>& vecAttrs, const vector<EAttrType>& vecPairs, EAttrType eExclude) const
{
  for (auto &v : vecAttrs)
  {
    if (v == eExclude)
      continue;

    auto o = find(vecPairs.begin(), vecPairs.end(), v);
    if (o != vecPairs.end())
      return true;
  }

  return false;
}

bool SEnchantCFG::isGoodEnchant(DWORD buffId, EItemType eType) const
{
  auto it = mapNoGoodEnchant.find(buffId);
  if (it == mapNoGoodEnchant.end())
    return true;
  
  const std::vector<EItemType>& rVec = it->second;
  
  for (auto &v : rVec)
  {
    if (eType == v)
      return false;
  }
  return true;
}

bool SEnchantCFG::isGoodEnchant(const Cmd::EnchantAttr& attr, EItemType eType, DWORD& dwPointOut) const
{
  const SEnchantAttr* pAttr = getEnchantAttr(attr.type(), eType);
  if(!pAttr)
    return false;

  if (RoleDataConfig::getMe().needSyncPercent(attr.type()))
    dwPointOut += (DWORD)(attr.value()*100/pAttr->fMax)/FLOAT_TO_DWORD;
  else
    dwPointOut += (DWORD)(attr.value()*100/pAttr->fMax);

  if (attr.value() < pAttr->fMax * pAttr->fExpressionOfMaxUp)
    return false;

  return true;
}

bool SEnchantCFG::gmCheckAndSet(EItemType eItemType, EnchantData& rData, DWORD buffId) const
{
  const SEnchantEquip* pEquip = getEnchantEquip(eItemType);
  if (pEquip == nullptr)
    return false;

  if (rData.attrs_size() <= 1)
    return false;
 
  vector<EAttrType> vecAttrs;
  vecAttrs.reserve(rData.attrs_size());
  for (int i = 0; i < rData.attrs_size(); ++i)
  {
    const EnchantAttr& rAttr = rData.attrs(i);
    vecAttrs.push_back(rAttr.type());
  }

  rData.clear_extras();
  for (auto &v : vecAttrs)
  {
    const SEnchantAttr* pAttr = getEnchantAttr(v, eItemType);
    if (pAttr == nullptr)
    {
      XERR << "附魔随机属性 : " << v << " 不合法" << XEND;
      continue;
    }
    const RoleData* pData = RoleDataConfig::getMe().getRoleData(v);
    if (pData == nullptr)
    {
      XERR << "附魔随机属性 : " << v << " 不合法" << XEND;
      continue;
    }

    if (contain(vecAttrs, pAttr->vecPairType, v) == true)
    {
      if (pAttr->canGetExtraBuff(eItemType))
      {
        if (pAttr->checkBuffId(buffId))
        {
          EnchantExtra* pExtra = rData.add_extras();
          if (pExtra)
          {
            pExtra->set_buffid(buffId);
            pExtra->set_configid(pAttr->dwConfigID);
            return true;
          }         
        }      
      }      
    }
  }
  return false;
}

const SEquipUpgradeCFG* SItemCFG::getUpgradeCFG(DWORD dwLevel) const
{
  auto m = mapUpgradeCFG.find(dwLevel);
  if (m != mapUpgradeCFG.end())
    return &m->second;
  return nullptr;
}

void SItemCFG::calcLockType()
{
  if(eLockType == 0)
    return;

  DWORD max = static_cast<DWORD>(EMANUALLOCKMETHOD_MAX);
  DWORD min = static_cast<DWORD>(EMANUALLOCKMETHOD_MIN);
  for(DWORD i=min; i<max; i++)
  {
    DWORD num = static_cast<DWORD>(pow(2,i));
    if( (num&eLockType) != 0 && EManualLockMethod_IsValid(i+1) == true)
      vecLockType.push_back(i+1);
  }
}

DWORD SItemCFG::getItemGetLimit(DWORD lv, ESource source, bool bMonthCard /*= false*/) const
{
  if (vecLimitSource.empty() == false)
  {
    auto it = std::find_if(vecLimitSource.begin(), vecLimitSource.end(), [source](ESource v) -> bool { return v == source; });
    if (it == vecLimitSource.end())
      return DWORD_MAX;
  }
  DWORD last = DWORD_MAX, extra = 0;
  for (auto& it : mapLevel2GetLimit)
  {
    last = it.second.first;
    extra = it.second.second;
    if (lv <= it.first)
      break;
  }
  return last + (bMonthCard ? extra : 0);
}

DWORD SItemCFG::getUseMultiple() const
{
  if (dwUseMultiple)
    return dwUseMultiple;
  const SItemErrorCFG* pCfg = ItemConfig::getMe().getItemErrorCFG(eItemType);
  if (pCfg == nullptr)
    return 0;
  return pCfg->dwUseNumber;
}

bool SItemCFG::isTimeValid() const
{
  DWORD dwNow = now();
  if (dwCardLotteryStartTime != 0 && dwNow < dwCardLotteryStartTime)
    return false;
  if (dwCardLotteryEndTime != 0 && dwNow > dwCardLotteryEndTime)
    return false;
  return true;
}

bool SItemCFG::isCardCompoaseTimeValid() const
{
  DWORD dwNow = now();
  if (dwCardComposeStartTime != 0 && dwNow < dwCardComposeStartTime)
    return false;
  if (dwCardComposeEndTime != 0 && dwNow > dwCardComposeEndTime)
    return false;
  return true;
}

/************************************************************************/
/*SLotteryCFG                                                                      */
/************************************************************************/

bool SLotteryItemCFG::isCurBatch() const
{
  DWORD curBatch = MiscConfig::getMe().getLotteryCFG().getCurBatch(dwItemType);
  if (curBatch == dwBatch)
    return true;
  return false;
}

DWORD SLotteryItemCFG::getRate() const
{
  if (eType == ELotteryType_Magic)
  {
    DWORD r = dwRate * MiscConfig::getMe().getLotteryCFG().getRate(dwItemType) / 10000;
    if (dwRate == 0 || MiscConfig::getMe().getLotteryCFG().getRate(dwItemType) == 0)
    {
      XERR << "[扭蛋-配置] 概率为0，id" << dwId << "itemid" << dwItemId << "dwRate" << dwRate << "itemtype" << dwItemType << XEND;
    }
    return r;
  }
  else 
  {
    return dwRate;
  }
}

bool SLotteryCFG::addItemCFG(const SLotteryItemCFG& rItemCFG)
{
  auto it = mapItemCFG.find(rItemCFG.dwId);
  if (it != mapItemCFG.end())
    return false;

  for (auto it : mapItemCFG)
  {
    if (it.second.strRarity.empty() == false && it.second.strRarity == rItemCFG.strRarity)
      return false;
  }
  mapItemCFG[rItemCFG.dwId] = rItemCFG;
  return true;
}

bool SLotteryCFG::random(DWORD& outItemId, DWORD &count, EGender gender, DWORD &rate) const
{
  outItemId = 0;
  DWORD rand = randBetween(0, dwTotalWeight);

  for (auto& m : mapItemCFG)
  {
    if (rand <= m.second.dwWeightOffSet && m.second.dwWeight != 0)
    {
      outItemId = m.second.getItemIdByGender(gender);
      count = m.second.dwCount;
      rate = m.second.dwRate;
      return true;
    }
  }
  return false;
}

bool SLotteryCFG::randomWithHideWeight(DWORD& outItemId, DWORD &count, EGender gender, DWORD &rate) const
{
  outItemId = 0;
  DWORD rand = randBetween(0, dwTotalHideWeight);

  for (auto& m : mapItemCFG)
  {
    if (rand <= m.second.dwHideWeightOffSet && m.second.dwHideWeight != 0)
    {
      outItemId = m.second.getItemIdByGender(gender);
      count = m.second.dwCount;
      rate = m.second.dwRate;
      return true;
    }
  }
  return false;
}

DWORD SLotteryCFG::needMoney()const
{
  DWORD dis = getDisCount();
  DWORD money = MiscConfig::getMe().getLotteryCFG().dwPrice;
  money = money - (money * dis/100);   
  return money;
}

DWORD SLotteryCFG::getDisCount() const
{
  if (isCurMonth())
  {
    return MiscConfig::getMe().getLotteryCFG().dwDiscount;
  }
  return 0;
}

bool SLotteryCFG::isCurMonth()const
{
  DWORD curSec = now();
  curSec -= 5 * 60 * 60;
  DWORD year = xTime::getYear(curSec);
  DWORD month = xTime::getMonth(curSec);
  if (year == dwYear && month == dwMonth)
    return true;
  return false;
}

/************************************************************************/
/* LotteryPool                                                          */
/************************************************************************/

void LotteryPool::add(SLotteryItemCFG& cfg)
{
  auto it = std::find(m_vecLotteryItemCFG.begin(), m_vecLotteryItemCFG.end(), cfg);
  if(m_vecLotteryItemCFG.end() != it)
  {
    XERR << "[LotteryPool] the same SLotteryItemCFG, id" << cfg.dwId << XEND;
    return;
  }

  m_vecLotteryItemCFG.push_back(cfg);
  XLOG << "[LotteryPool] add Success, itemId" << cfg.getItemId() << "femaleItemId" << cfg.dwFemaleItemId << "cost" << cfg.dwBasePrice << "count" << cfg.dwCount << XEND;
}

bool LotteryPool::get(DWORD& dwOutItem, DWORD& dwOutCount, DWORD& dwOutRate, EGender gender, DWORD dwCost, TVecDWORD& vecItemDels)
{
  std::vector<SLotteryItemCFG> vecItemValids;
  for(auto& v : m_vecLotteryItemCFG)
  {
    if(dwCost < v.dwBasePrice)
      continue;

    auto it = std::find(vecItemDels.begin(), vecItemDels.end(), v.getItemIdByGender(gender));
    if(vecItemDels.end() != it)
      continue;

    vecItemValids.push_back(v);
  }

  if(0 >= vecItemValids.size())
    return false;

  SLotteryItemCFG* st = randomStlContainer(vecItemValids);
  if(!st)
    return false;

  dwOutItem = st->getItemIdByGender(gender);
  dwOutCount = st->dwCount;
  dwOutRate = st->dwRate;
  return true;
}


bool SHeadEffect::checkCond(SHeadCondType condType, const TVecQWORD& vecParam/* = TVecQWORD{}*/) const
{
  if (oCond.condType != condType)
    return false;
  
  if (vecParam.empty() && oCond.vecParams.empty())
    return true;

  for (auto&v : vecParam)
  {
    auto it = std::find(oCond.vecParams.begin(), oCond.vecParams.end(), v);
    if (it != oCond.vecParams.end())
      return true;
  }

  return false;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
// config
ItemConfig::ItemConfig()
{

}

ItemConfig::~ItemConfig()
{

}

bool ItemConfig::loadConfig()
{
  bool bResult = true;
  if (loadItemConfig() == false)
    bResult = false;
  if (loadStuffConfig() == false)
    bResult = false;
  if (loadEquipConfig() == false)
    bResult = false;
  if (loadEquipLotteryConfig() == false)
    bResult = false;
  if (loadCardConfig() == false)
    bResult = false;
  if (loadMountConfig() == false)
    bResult = false;
  if (loadMasterConfig() == false)
    bResult = false;
  if (loadSuitConfig() == false)
    bResult = false;
  if (loadRefineConfig() == false)
    bResult = false;
  // 2016-04-19 申林移除Table_EquipFashion.txt表
  //if (loadFashionConfig() == false)
  //  bResult = false;
  if (loadItemOriginConfig() == false)
    bResult = false;
  if (loadItemType() == false)
    bResult = false;
  if (loadEnchantConfig() == false)
    bResult = false; 
  if (loadHairStyle() == false)
    bResult = false;
  if (loadAppellationConfig() == false)
    bResult = false;
  //if (loadTitleConfig() == false)
  //  bResult = false;
  if (loadEquipUpgradeConfig() == false)
    bResult = false;
  if (loadEquipDecomposeConfig() == false)
    bResult = false;

  //warning:loadEquipUpgradeConfig 必须要在loadExchangeConfig前面
  if (loadExchangeConfig() == false)
    bResult = false;
  if (loadEnchantPriceConfig() == false)
    bResult = false;
  if (loadCardRateConfig() == false)
    bResult = false;
  if (loadLotteryConfig() == false)
    bResult = false;
  if (loadHeadEffectConfig() == false)
    bResult = false;
  if (loadPackSlotLvConfig() == false)
    bResult = false;
  if (loadLotteryGiveConfig() == false)
    bResult = false;
  if (loadPickEffectConfig() == false)
    bResult = false;
  if (loadEquipGenderConfig() == false)
    bResult = false;
  bResult &= loadEquipComposeConfig();

  return bResult;
}

bool ItemConfig::checkConfig()
{
  bool bCorrect = true;
  for (auto m = m_mapItemCFG.begin(); m != m_mapItemCFG.end(); ++m)
  {
    // manual item
    for (auto v = m->second.vecManualItems.begin(); v != m->second.vecManualItems.end(); ++v)
    {
      if (v->id() != 0 && getItemCFG(v->id()) == nullptr)
      {
        bCorrect = false;
        XERR << "[物品配置-冒险手册道具] itemid : " << m->first << " id : " << v->id() << " 未在Table_Item.txt表中找到" << XEND;
      }
    }

    // equip
    if (m->second.dwSubID != 0 && ComposeConfig::getMe().getComposeCFG(m->second.dwSubID) == nullptr)
    {
      bCorrect = false;
      XERR << "[物品配置-装备] itemid :" << m->first << "subid :" << m->second.dwSubID << "未在Table_Compose.txt表中找到" << XEND;
    }
    if (m->second.dwUpgID != 0 && ComposeConfig::getMe().getComposeCFG(m->second.dwUpgID) == nullptr)
    {
      bCorrect = false;
      XERR << "[物品配置-装备] itemid :" << m->first << "upgid :" << m->second.dwUpgID << "未在Table_Compose.txt表中找到" << XEND;
    }
    // buff
    for (auto &v : m->second.vecBuffIDs)
    {
      if (TableManager::getMe().getBufferCFG(v) == nullptr)
      {
        bCorrect = false;
        XERR << "[物品配置-Buff], itemid: " << m->first << "buffid: " << v << "未在Table_Buffer.txt中找到" << XEND;
      }
    }
    for (auto &v : m->second.vecAdvBuffIDs)
    {
      if (TableManager::getMe().getBufferCFG(v) == nullptr)
      {
        bCorrect = false;
        XERR << "[物品配置-Buff], itemid: " << m->first << "buffid: " << v << "未在Table_Buffer.txt中找到" << XEND;
      }
    }
    for (auto &v : m->second.vecStoreBuffIDs)
    {
      if (TableManager::getMe().getBufferCFG(v) == nullptr)
      {
        bCorrect = false;
        XERR << "[物品配置-Buff], itemid: " << m->first << "buffid: " << v << "未在Table_Buffer.txt中找到" << XEND;
      }
    }
    for (auto &p : m->second.vecRefine2Buff)
    {
      for (auto &v : p.second)
      {
        if (TableManager::getMe().getBufferCFG(v) == nullptr)
        {
          bCorrect = false;
          XERR << "[物品配置-Buff], itemid: " << m->first << "buffid: " << v << "未在Table_Buffer.txt中找到" << XEND;
        }
      }
    }

    // equip upgrade
    TMapEquipUpgradeCFG& mapCFG = m->second.mapUpgradeCFG;
    for (auto &upgrade : mapCFG)
    {
      for (auto &v : upgrade.second.vecMaterial)
      {
        if (getItemCFG(v.id()) == nullptr)
        {
          bCorrect = false;
          XERR << "[物品配置-装备升级] id :" << m->first << "level :" << upgrade.first << "material :" << v.id() << "未在Table_Item.txt表中找到" << XEND;
          continue;
        }
      }
      if (upgrade.second.oProduct.id() != 0 && getItemCFG(upgrade.second.oProduct.id()) == nullptr)
      {
        bCorrect = false;
        XERR << "[物品配置-装备升级] id :" << m->first << "level :" << upgrade.first << "product :" << upgrade.second.oProduct.ShortDebugString() << "未在Table_Item.txt表中找到" << XEND;
        continue;
      }
      if (upgrade.second.dwBuffID != 0 && TableManager::getMe().getBufferCFG(upgrade.second.dwBuffID) == nullptr)
      {
        bCorrect = false;
        XERR << "[物品配置-装备升级] id: " << m->first << "level :" << upgrade.first << "buffid: " << upgrade.second.dwBuffID << "未在Table_Buffer.txt中找到" << XEND;
        continue;
      }
      if (NpcConfig::getMe().getNpcCFG(upgrade.second.dwNpcID) == nullptr)
      {
        bCorrect = false;
        XERR << "[物品配置-装备升级] id: " << m->first << "level :" << upgrade.first << "npcid: " << upgrade.second.dwNpcID << "未在Table_Npc.txt中找到" << XEND;
        continue;
      }
    }

    //food
    if (m->second.eItemType == EITEMTYPE_FOOD)
    {
      const SFoodConfg *pCfg = FoodConfig::getMe().getFoodCfg(m->first);
      if (!pCfg)
      {
        bCorrect = false;
        XERR << "[物品配置-料理], itemid: " << m->first << "未在Table_Food.txt中找到" << XEND;
      }
    }
  }

  for (int i = EENCHANTTYPE_MIN + 1; i < EENCHANTTYPE_MAX; ++i)
  {
    SEnchantCFG& rCFG = m_arrEnchantCFG[i];
    if (rCFG.oNeedItem.id() != 0 && ItemConfig::getMe().getItemCFG(rCFG.oNeedItem.id()) == nullptr)
    {
      bCorrect = false;
      XERR << "[物品配置-附魔] type : " << i << " itemid : " << rCFG.oNeedItem.id() << " 未在 Table_Item.txt 表中找到" << XEND;
    }
  }

  for (auto m = m_mapExchangeItemCFG.begin(); m != m_mapExchangeItemCFG.end(); m++)
  {
    SExchangeItemCFG& stCFG = m->second;
    const STradeItemTypeData* pData = TableManager::getMe().getTradeItemCFG(stCFG.dwCategory);
    if (pData)
    {
      DWORD dwType = pData->getType();
      if (dwType == 0)
      {
        stCFG.dwBigCategory = stCFG.dwCategory;
        stCFG.dwShowOrdcer = pData->getExchangeOrder();
      }
      else {
        stCFG.dwBigCategory = dwType;
        const STradeItemTypeData* pFahterData = TableManager::getMe().getTradeItemCFG(stCFG.dwBigCategory);
        if (pFahterData)
        {
          stCFG.dwShowOrdcer = pFahterData->getExchangeOrder();
        }
      }
    }
  }

  // suit config
  for (auto &m : m_mapSuitCFG)
  {
    for (auto &n : m.second.vecBuffs)
    {
      for (auto &v : n.second)
      {
        if (TableManager::getMe().getBufferCFG(v) == nullptr)
        {
          bCorrect = false;
          XERR << "[套装配置-Buff], suitid: " <<  m.first << "buffid: " << v << "未在Table_Buffer.txt中找到" << XEND;
        }
      }
    }
  }
  for (int i = EENCHANTTYPE_MIN + 1; i < EENCHANTTYPE_MAX; ++i)
  {
    for (auto &m : m_arrEnchantCFG[i].vecAttrs)
    {
      for (auto &v : m.vecExtraBuff)
      {
        if (TableManager::getMe().getBufferCFG(v.first) == nullptr)
        {
          bCorrect = false;
          XERR << "[装备附魔-Buff], UniqID: " << m.dwConfigID << "buffid: " << v.first << "未在Table_Buffer.txt中找到" << XEND;
        }
      }
    }
  }

  //trade
  for (auto& v : m_mapExchangeItemCFG)
  {
    if (v.second.minPrice.type == MINPRICETYPE_CARD_COMPOSE ||
      v.second.minPrice.type == MINPRICETYPE_EQUIP_COMPOSE )
    {
      const SComposeCFG* pComposeCFG = ComposeConfig::getMe().getComposeCFG(v.second.minPrice.dwComposeId);
      if (pComposeCFG == nullptr)
      {
        bCorrect = false;
        XERR << "[交易所配置-minprice], UniqID: " << v.first << "composeid " << v.second.minPrice.dwComposeId << "未在Table_Compose.txt中找到" << XEND;
        continue;
      }
      v.second.minPrice.vecMaterial = pComposeCFG->vecMaterial;
      v.second.minPrice.dwRob = pComposeCFG->dwROB;
    }
    else if (v.second.minPrice.type == MINPRICETYPE_EQUIP_UPGRADE)
    {
      //check todo
      const SItemCFG *pItemCfg = getItemCFG(v.second.minPrice.dwEquipUpgrade);
      if (pItemCfg == nullptr)
      {
        bCorrect = false;
        XERR << "[交易所配置-minprice], UniqID: " << v.first << "itemid" << v.second.minPrice.dwEquipUpgrade << "未在Table_Item.txt中找到" << XEND;
        continue;
      }
      const SEquipUpgradeCFG* pUpgradeCFG = pItemCfg->getUpgradeCFG(1);
      if (pUpgradeCFG == nullptr)
      {
        bCorrect = false;
        XERR << "[交易所配置-minprice], UniqID: " << v.first << "itemid" << v.second.minPrice.dwEquipUpgrade<<"lv 1" << "未在Table_EquipUpgrade.txt中找到" << XEND;
        continue;
      }
      
      if (pUpgradeCFG->oProduct.id() != v.second.dwId)
      {
        bCorrect = false;
        XERR << "[交易所配置-minprice-重大错误], UniqID: " << v.first << "itemid" << v.second.minPrice.dwEquipUpgrade << "Productid"<<pUpgradeCFG->oProduct.id() << "和交易所equip_upgrade_id不一致" << XEND;
        continue;
      }
    }

    //check
    for (auto&v1 : v.second.vecPriceSum)
    {
      const SExchangeItemCFG* p = getExchangeItemCFG(v1.first);
      if (!p)
      {
        bCorrect = false;
        XERR << "[交易所配置-price], UniqID: " << v.first << "itemid: " <<v1.first << "未在Table_Exchange.txt中找到" << XEND;
        continue;
      }
    }
    if (v.second.minPrice.dwEquipId)
    {
      const SExchangeItemCFG* p = getExchangeItemCFG(v.second.minPrice.dwEquipId);
      if (!p)
      {
        bCorrect = false;
        XERR << "[交易所配置-minprice], UniqID: " << v.first << "合成表里的equipid " << v.second.minPrice.dwEquipId << "未在Table_Exchange.txt中找到" << XEND;
      }
    }

    for (auto &v2 : v.second.minPrice.vecMaterial)
    {
      const SExchangeItemCFG* p = getExchangeItemCFG(v2.id());
      if (!p)
      {
        bCorrect = false;
        XERR << "[交易所配置-minprice], UniqID: " << v.first << "合成表里的材料未在: " << v2.id() << "未在Table_Exchange.txt中找到" << XEND;
        continue;
      }
    }
  }

  // equip decompose
  for (auto &m : m_mapDecomposeCFG)
  {
    for (auto &v : m.second.vecMaterial)
    {
      if (getItemCFG(v.id()) == nullptr)
      {
        m.second.blInit = bCorrect = false;
        XERR << "[物品配置-装备熔炼] id :" << m.first << "material :" << v.ShortDebugString() << "未在Table_Item.txt表中找到" << XEND;
        continue;
      }
    }
  }

  // card rate
  for (auto &v : m_vecCardRateCFG)
  {
    for (auto &item : v.vecReward)
    {
      if (getItemCFG(item.id()) == nullptr)
      {
        bCorrect = false;
        XERR << "[物品配置-卡片概率] itemid :" << item.id() << "未在 Table_Item.txt 表中找到" << XEND;
        continue;
      }
    }
  }

  ////for test debug
  //const SEnchantCFG* pCFG = getEnchantCFG(EENCHANTTYPE_SENIOR);
  //if (pCFG)
  //{
  //  for (int i = 0; i < 10000; i++)
  //  {
  //    EnchantData rData;
  //    pCFG->random(EITEMTYPE_WEAPON_KNIFE, rData);
  //    
  //    std::stringstream ss;
  //    ss << "times:" << i;
  //    for (int j = 0; j < rData.attrs_size(); j++)
  //    {
  //      EnchantAttr attr = rData.attrs(j);
  //      ss << "attr:" << attr.type() <<"value:"<<attr.value();
  //    }
  //    XLOG("[randomtest_debug] %s", ss.str().c_str());
  //  }
  //} 
  return bCorrect;
}

bool ItemConfig::canEnterManual(const SItemCFG* pCFG) const
{
  if (pCFG == nullptr)
    return false;
  if (pCFG->swAdventureValue == 0)
    return false;

  return true;
}

const SItemCFG* ItemConfig::getItemCFG(DWORD dwTypeID) const
{
  auto m = m_mapItemCFG.find(dwTypeID);
  if (m != m_mapItemCFG.end())
    return &m->second;

  return nullptr;
}

const SItemCFG* ItemConfig::getHairCFG(DWORD dwHairID) const
{
  auto m = m_mapHairID2ItemID.find(dwHairID);
  if (m == m_mapHairID2ItemID.end())
    return nullptr;
  return getItemCFG(m->second);
}

const SExchangeItemCFG* ItemConfig::getExchangeItemCFG(DWORD id) const
{
  auto m = m_mapExchangeItemCFG.find(id);
  if (m != m_mapExchangeItemCFG.end())
    return &m->second;
  return nullptr;
}

const SEquipDecomposeCFG* ItemConfig::getEquipDecomposeCFG(DWORD dwID) const
{
  auto m = m_mapDecomposeCFG.find(dwID);
  if (m != m_mapDecomposeCFG.end())
    return &m->second;
  return nullptr;
}

const SItemCFG* ItemConfig::randCardByQuality(EQualityType eType) const
{
  auto m = m_mapQualityCard.find(eType);
  if (m == m_mapQualityCard.end())
    return nullptr;
  DWORD dwRand = randBetween(0, m->second.dwMaxWeight);
  for (auto &v : m->second.vecCardCFG)
  {
    if (dwRand <= v.dwCardWeight)
      return &v;
  }
  return nullptr;
}

const SCardRateCFG* ItemConfig::getCardRateCFG(const vector<EQualityType>& vecQuality) const
{
  if (vecQuality.size() != 3)
    return nullptr;

  for (auto &v : m_vecCardRateCFG)
  {
    if (v.vecQuality[0] == vecQuality[0] && v.vecQuality[1] == vecQuality[1] && v.vecQuality[2] == vecQuality[2])
      return &v;
  }
  return nullptr;
}

const SQualityCard* ItemConfig::getCardQualityCFG(EQualityType eType) const
{
  auto m = m_mapQualityCard.find(eType);
  if (m != m_mapQualityCard.end())
    return &m->second;
  return nullptr;
}

//DWORD ItemConfig::getExchangePrice(DWORD id, DWORD refineLv) const
//{
//  const SExchangeItemCFG *pBase = getExchangeItemCFG(id);
//  if (pBase == nullptr)
//    return 0 ;
//  
//  if (pBase->dwPriceType == PRICETYPE_SUM)
//  {
//    DWORD dwServerPrice = 0;
//    for (auto&v : pBase->vecPriceSum)
//    {      
//      dwServerPrice += getExchangePrice(v.first, 0) * v.second;
//    }
//    return dwServerPrice;
//  }
//  if (refineLv == 0)
//    return pBase->dwPrice;
//  
//  const SItemCFG* pItemCFG = getItemCFG(id);
//  if (pItemCFG == nullptr)
//  {
//    XERR << "[交易-服务器价格] 失败，获取不到SItemCFG 策划表 itemid：" << id << XEND;
//    return 0;
//  }
//
//  if (pItemCFG->eEquipType <= EEQUIPTYPE_MIN || pItemCFG->eEquipType >= EEQUIPTYPE_MAX)
//  {
//    XERR << "[交易-服务器价格] 失败，装备类型错误 itemid：" << id << "equiptype：" << pItemCFG->eEquipType << XEND;
//    return 0;
//  }
//
//  const SRefineTradeCFG* pRefineTradeCFG = ItemConfig::getMe().getRefineTradeCFG(pItemCFG->eItemType, refineLv);
//  if (pRefineTradeCFG == nullptr)
//  {
//    XERR << "[交易-服务器价格] 失败，获取不到pRefineTradeCFG 策划表 itemtype:" << pItemCFG->eItemType << "refinelv：" << refineLv << XEND;
//    return 0;
//  }
//  DWORD otherItemid = pRefineTradeCFG->dwItemID;
//  DWORD otherPrice0 = getExchangePrice(otherItemid, 0);
//  return pBase->dwPrice*pRefineTradeCFG->dwEquipRate + otherPrice0*pRefineTradeCFG->dwItemRate + pRefineTradeCFG->dwLvRate;
//}

const SMasterCFG* ItemConfig::getMasterCFG(EMasterType eType, DWORD lv) const
{
  auto v = find_if(m_vecMasterCFG.rbegin(), m_vecMasterCFG.rend(), [eType, lv](const SMasterCFG& r) -> bool{
    return r.eType == eType && lv >= r.dwLv;
  });
  if (v == m_vecMasterCFG.rend())
    return nullptr;

  return &(*v);
}

const SSuitCFG* ItemConfig::getSuitCFG(DWORD id) const
{
  auto it = m_mapSuitCFG.find(id);
  if (it == m_mapSuitCFG.end())
    return nullptr;
  return &(it->second);
}

const SRefineCFG* ItemConfig::getRefineCFG(EItemType eType, EQualityType eQuality, DWORD lv) const
{
  auto v = find_if(m_vecRefineCFG.begin(), m_vecRefineCFG.end(), [eType, eQuality, lv](const SRefineCFG& r) -> bool{
    return r.eItemType == eType && r.dwRefineLv == lv && r.sComposeRate.eQuaType == eQuality;
  });
  if (v == m_vecRefineCFG.end())
    return nullptr;
  return &(*v);
}

const SRefineTradeCFG* ItemConfig::getRefineTradeCFG(EItemType eType, DWORD lv) const
{
  auto v = find_if(m_vecRefineTradeCFG.begin(), m_vecRefineTradeCFG.end(), [eType, lv](const SRefineTradeCFG& r) -> bool {
    return r.eItemType == eType && r.dwRefineLv == lv;
  });
  if (v == m_vecRefineTradeCFG.end())
    return nullptr;
  return &(*v);
}

const SItemErrorCFG* ItemConfig::getItemErrorCFG(EItemType eType) const
{
  auto m = m_mapType2Error.find(eType);
  if (m != m_mapType2Error.end())
    return &m->second;

  return nullptr;
}

const SEnchantCFG* ItemConfig::getEnchantCFG(EEnchantType eType) const
{
  if (eType <= EENCHANTTYPE_MIN || eType >= EENCHANTTYPE_MAX || EEnchantType_IsValid(eType) == false)
    return nullptr;

  return &m_arrEnchantCFG[eType];
}

const TSetDWORD& ItemConfig::getCDGroup(DWORD dwGroup) const
{
  const static TSetDWORD e;
  auto m = m_mapCDGroupCFG.find(dwGroup);
  if (m != m_mapCDGroupCFG.end())
    return m->second;

  return e;
}

const TSetDWORD& ItemConfig::getSuitIDs(DWORD equipTypeID) const
{
  const static TSetDWORD vec;
  auto m = m_mapEquipID2SuitID.find(equipTypeID);
  if (m == m_mapEquipID2SuitID.end())
    return vec;

  return m->second;
}

DWORD ItemConfig::getCardIDByMonsterID(DWORD monsterid) const
{
  auto v = find_if(m_vecMonster2Card.begin(), m_vecMonster2Card.end(), [monsterid](const TPairMonsterCard& r) -> bool{
    return r.first == monsterid;
  });
  if (v == m_vecMonster2Card.end())
    return 0;

  return v->second;
}

float ItemConfig::getEnchantPriceRate(DWORD attr, DWORD itemId) const
{
  const SItemCFG* pCfg = getItemCFG(itemId);
  if (!pCfg)
    return 1;
  auto it = m_mapEnchantPriceCFG.find(attr);
  if (it == m_mapEnchantPriceCFG.end())
    return 1;

  auto subIt = it->second.mapValue.find(pCfg->eItemType);
  if (subIt == it->second.mapValue.end())
    return 1;
  float f = subIt->second;
  XDBG << "[交易-附魔] 相性" << "attr" << attr << "itemid" << itemId <<"type"<<pCfg->eItemType << "相性" << f << XEND;
  return f;
}

DWORD ItemConfig::getPackSlotLvCFG(DWORD dwLv, EPackType eType) const
{
  if (eType != EPACKTYPE_MAIN && eType != EPACKTYPE_PERSONAL_STORE)
    return 0;

  for (auto m = m_mapLv2PackSlot.rbegin(); m != m_mapLv2PackSlot.rend(); ++m)
  {
    if (!m->second.blInit)
      continue;
    if (dwLv >= m->second.dwBaseLv)
      return eType == EPACKTYPE_MAIN ? m->second.dwMainSlot : m->second.dwPStoreSlot;
  }

  return 0;
}

bool ItemConfig::loadItemConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Item.txt"))
  {
    XERR << "[物品配置] 加载配置Table_Item.txt失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Item", table);
  m_mapItemCFG.clear();
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    // check dupcliated
    if (getItemCFG(m->first) != nullptr)
    {
      XERR << "[物品配置] id :" << m->first << "重复了!" << XEND;
      bCorrect = false;
      continue;
    }

    // check quality
    DWORD dwQuality = m->second.getTableInt("Quality");
    if (dwQuality <= EQUALITYTYPE_MIN || dwQuality >= EQUALITYTYPE_MAX || EQualityType_IsValid(dwQuality) == false)
    {
      XERR << "[物品配置] id :" << m->first << "Quality :" << dwQuality << "不合法" << XEND;
      bCorrect = false;
      continue;
    }

    // check itemtype
    DWORD dwItemType = m->second.getTableInt("Type");
    //if (dwItemType == 148 || dwItemType == 149)
    if (dwItemType == 148)
      continue;
    if (dwItemType <= EITEMTYPE_MIN || dwItemType >= EITEMTYPE_MAX || EItemType_IsValid(dwItemType) == false)
    {
      XERR << "[物品配置] id :" << m->first << "Type :" << dwItemType << "不合法" << XEND;
      bCorrect = false;
      continue;
    }

    // check max num
    DWORD dwMaxNum = m->second.getTableInt("MaxNum");
    if (isPackCheck(static_cast<EItemType>(dwItemType)) == true && dwMaxNum == 0)
    {
      XERR << "[物品配置] id :" << m->first << "MaxNum :" << dwMaxNum << "不合法" << XEND;
      bCorrect = false;
      continue;
    }

    // check locktype
    /*
    DWORD dwLockType = m->second.getTableInt("Condition");
    if (EManualLockMethod_IsValid(dwLockType) == false)
    {
      XERR << "[物品配置] id :" << m->first << "Condition :" << dwLockType << "不合法" << XEND;
      bCorrect = false;
      continue;
    }
    */

    // create config data
    SItemCFG stCFG;
    stCFG.dwTypeID = m->first;
    stCFG.dwMaxNum = m->second.getTableInt("MaxNum");
    stCFG.dwSellPrice = m->second.getTableInt("SellPrice");
    stCFG.dwComposeID = m->second.getTableInt("ComposeID");
    stCFG.dwLevel = m->second.getTableInt("Level");
    stCFG.eItemType = static_cast<EItemType>(dwItemType);
    stCFG.eQualityType = static_cast<EQualityType>(dwQuality);
    stCFG.swAdventureValue = m->second.getTableInt("AdventureValue");
    stCFG.dwItemSort = m->second.getTableInt("ItemSort");
    stCFG.dwInHandLmt = m->second.getTableInt("InHandLmt");
    stCFG.dwHandInLmt = m->second.getTableInt("HandInLmt");
    stCFG.strNameZh = m->second.getTableString("NameZh");

    //stCFG.eLockType = static_cast<EManualLockMethod>(m->second.getTableInt("Condition"));
    stCFG.eLockType = m->second.getTableInt("Condition");
    stCFG.calcLockType();

    /*
    xLuaData& condition = m->second.getMutableData("Condition");
    xLuaData& lockType = condition.getMutableData("LockType");
    auto lockf = [&](const string& str, xLuaData& data)
    {
      stCFG.vecLockType.push_back(data.getInt());
    };
    lockType.foreach(lockf);
    */

    xLuaData& itemTarget = m->second.getMutableData("ItemTarget");
    stCFG.eTargetType = static_cast<ETragetType>(itemTarget.getTableInt("type"));
    stCFG.dwRange = itemTarget.getTableInt("range");
    stCFG.dwNoStoreage = m->second.getTableInt("NoStorage");
    stCFG.bNoSale = m->second.getTableInt("NoSale") == 1;

    // adventure
    xLuaData& adventure = m->second.getMutableData("AdventureReward");
    xLuaData& advbuffid = adventure.getMutableData("buffid");
    auto advbuffidf = [&](const string& str, xLuaData& data)
    {
      stCFG.vecAdvBuffIDs.push_back(data.getInt());
    };
    advbuffid.foreach(advbuffidf);
    xLuaData& advrewardid = adventure.getMutableData("rewardid");
    auto advrewardidf = [&](const string& str, xLuaData& data)
    {
      stCFG.setAdvRewardIDs.insert(data.getInt());
    };
    advrewardid.foreach(advrewardidf);
    // StorageReward
    xLuaData& storage = m->second.getMutableData("StorageReward");
    xLuaData& srotebuffid = storage.getMutableData("buffid");
    auto storebuffidf = [&](const string& str, xLuaData& data)
    {
      stCFG.vecStoreBuffIDs.push_back(data.getInt());
    };
    srotebuffid.foreach(storebuffidf);
    xLuaData& manualitem = adventure.getMutableData("item");
    auto manualitemf = [&](const string& str, xLuaData& data)
    {
      ItemInfo oItem;
      oItem.set_id(data.getTableInt("1"));
      oItem.set_count(data.getTableInt("2"));
      oItem.set_source(ESOURCE_MANUAL);
      stCFG.vecManualItems.push_back(oItem);
    };
    manualitem.foreach(manualitemf);

    // profession req
    xLuaData& profession = m->second.getMutableData("JobLimit");
    auto professionf = [&](const string& key, xLuaData& data)
    {
      DWORD profession = data.getInt();
      if (profession == 0)
        return;
      if (profession <= EPROFESSION_MIN || profession >= EPROFESSION_MAX || EProfession_IsValid(profession) == false)
      {
        bCorrect = false;
        XERR << "[物品配置-使用道具] id : " << m->first << " 使用道具职业 " << profession << " 不是合法的职业" << XEND;
        return;
      }
      stCFG.vecProLimit.push_back(static_cast<EProfession>(profession));
    };
    profession.foreach(professionf);

    DWORD deltype = m->second.getTableInt("ExistTimeType");
    const string& existtime = m->second.getTableString("ExistTime");
    if (deltype > EITEMDELTYPE_MIN && deltype < EITEMDELTYPE_MAX)
    {
      stCFG.eDelType = static_cast<EItemDelType>(deltype);
      if (stCFG.eDelType == EITEMDELTYPE_TIME)
      {
        stCFG.dwDelTime = atoi(existtime.c_str());
      }
      else if (stCFG.eDelType == EITEMDELTYPE_DATE && existtime.empty() == false)
      {
        parseTime(existtime.c_str(), stCFG.dwDelDate);
      }
    }

    stCFG.dwFeature = m->second.getTableInt("Feature");

    xLuaData& getlimit = m->second.getMutableData("GetLimit");
    if (getlimit.has("type"))
    {
      EItemGetLimitType type = static_cast<EItemGetLimitType>(getlimit.getTableInt("type"));
      switch (type)
      {
      case EITEMGETLIMITTYPE_DAY:
      case EITEMGETLIMITTYPE_WEEK:
      {
        stCFG.eGetLimitType = type;
        if (getlimit.has("source"))
        {
          auto lmtsrcf = [&](const string& str, xLuaData& data)
            {
              DWORD source = data.getInt();
              if (source <= ESOURCE_MIN || source >= ESOURCE_MAX || ESource_IsValid(source) == false)
              {
                bCorrect = false;
                XERR << "[物品配置] id :" << m->first << "获取上限source:" << source << "非法" << XEND;
                return;
              }
              stCFG.vecLimitSource.push_back(static_cast<ESource>(source));
            };
          getlimit.getMutableData("source").foreach(lmtsrcf);
        }
        xLuaData& limit = getlimit.getMutableData("limit");
        auto getlimitf = [&](const string& str, xLuaData& data)
          {
            stCFG.mapLevel2GetLimit[atoi(str.c_str())].first = data.getTableInt("1");
            stCFG.mapLevel2GetLimit[atoi(str.c_str())].second = data.getTableInt("2");
          };
        limit.foreach(getlimitf);
        stCFG.bUniformSource = getlimit.getTableInt("uniform_source") == 1;
        break;
      }
      default:
        bCorrect = false;
        XERR << "[物品配置] id :" << m->first << "获取上限type:" << type << "非法" << XEND;
        break;
      }
    }

    // insert to list
    m_mapItemCFG[m->first] = stCFG;
  }

  if(bCorrect)
    XLOG << "[物品配置] 成功加载配置Table_Item.txt" << XEND;
  return bCorrect;
}

bool ItemConfig::loadStuffConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_UseItem.txt"))
  {
    XERR << "[物品配置-使用道具] 加载配置Table_UseItem.txt失败" << XEND;
    return false;
  }

  m_setItemSkills.clear();
  m_mapCDGroupCFG.clear();
  m_vecGenderReward.clear();
  m_mapPetWearIDs.clear();

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_UseItem", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    // get item config
    auto item = m_mapItemCFG.find(m->first);
    if (item == m_mapItemCFG.end())
    {
      XERR << "[物品配置-使用道具] id : " << m->first << " 未在Table_Item.txt表中找到" << XEND;
      bCorrect = false;
      continue;
    }

    // check item type
    if (isUseItem(item->second.eItemType) == false)
    {
      XERR << "[物品配置-使用道具] id : " << m->first << " 类型 不是使用形道具" << XEND;
      bCorrect = false;
      continue;
    }

    // init data
    item->second.dwCD = m->second.getTableInt("CDTime") * ONE_THOUSAND;
    if(m->second.has("PVPCDtime"))
      item->second.dwPVPCD = m->second.getTableInt("PVPCDtime") * ONE_THOUSAND;
    else
      item->second.dwPVPCD = item->second.dwCD;
    item->second.dwTransformLmt = m->second.getTableInt("ChangeLimit");
    item->second.dwUsedSys = m->second.getTableInt("UsedSys");
    item->second.dwTargeUsedSys = m->second.getTableInt("TargetSys");    
    item->second.dwFailSys = m->second.getTableInt("FailSys");
    item->second.oGMData = m->second.getMutableData("UseEffect");
    item->second.dwCDGroup = m->second.getTableInt("CDGroup");
    item->second.dwDaliyLimit = m->second.getTableInt("DailyLimit");
    item->second.dwWeekLimit = m->second.getTableInt("WeekLimit");
    item->second.dwUseLimit = m->second.getTableInt("UseLimit");
    item->second.dwUseMultiple = m->second.getTableInt("UseMultiple");

    const string& starttime = m->second.getTableString("UseStartTime");
    if (!starttime.empty())
      parseTime(starttime.c_str(), item->second.dwUseStartTime);
    const string& endtime = m->second.getTableString("UseEndTime");
    if (!endtime.empty())
      parseTime(endtime.c_str(), item->second.dwUseEndTime);
    if ((item->second.dwUseStartTime || item->second.dwUseEndTime) && item->second.dwUseStartTime >= item->second.dwUseEndTime)
      XERR << "[物品配置-使用道具]" << item->first << "使用开始时间:" << starttime << "使用结束时间:" << endtime << "非法" << XEND;

    // profession req
    xLuaData& profession = m->second.getMutableData("Class");
    auto professionf = [&](const string& key, xLuaData& data)
    {
      DWORD profession = data.getInt();
      if (profession == 0)
        return;
      if (profession <= EPROFESSION_MIN || profession >= EPROFESSION_MAX || EProfession_IsValid(profession) == false)
      {
        bCorrect = false;
        XERR << "[物品配置-使用道具] id : " << item->first << " 穿戴职业 " << profession << " 不是合法的职业" << XEND;
        return;
      }
      item->second.vecEquipPro.push_back(static_cast<EProfession>(profession));
    };
    profession.foreach(professionf);

    if (strncmp(item->second.oGMData.getTableString("type"), "client_useskill", 15) == 0)
    {
      m_setItemSkills.insert(item->second.oGMData.getTableInt("id"));
    }
    if (item->second.dwCDGroup != 0)
      m_mapCDGroupCFG[item->second.dwCDGroup].insert(item->second.dwTypeID);

    xLuaData& rGMData = item->second.oGMData;
    const string& action = rGMData.getTableString("type");
    if (action == "genderreward")
    {
      pair<DWORD, DWORD> p;
      p.first = rGMData.getTableInt("male");
      p.second = rGMData.getTableInt("female");
      m_vecGenderReward.push_back(p);
    }
    else if (action == "unlockpetwear")
    {
      m_mapPetWearIDs[item->second.eQualityType].insert(item->first);
    }
  }

  if (bCorrect)
    XLOG << "[物品配置-使用道具] 成功加载配置Table_UseItem.txt" << XEND;
  return bCorrect;
}

bool ItemConfig::loadEquipConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Equip.txt"))
  {
    XERR << "[物品配置-装备] 加载配置Table_Equip.txt失败" << XEND;
    return false;
  }

  m_mapBaseVID2Equips.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Equip", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    // get item config
    auto item = m_mapItemCFG.find(m->first);
    if (item == m_mapItemCFG.end())
    {
      XERR << "[物品配置-装备] id : " << m->first << " 未在Table_Item.txt表中找到" << XEND;
      bCorrect = false;
      continue;
    }

    // init data
    DWORD dwEquipType = m->second.getTableInt("EquipType");
    if (dwEquipType <= EEQUIPTYPE_MIN || dwEquipType >= EEQUIPTYPE_MAX || EEquipType_IsValid(dwEquipType) == false)
    {
      XERR << "[物品配置] id :" << m->first << "EquipType :" << dwEquipType << "不合法" << XEND;
      bCorrect = false;
      continue;
    }
    item->second.eEquipType = static_cast<EEquipType>(dwEquipType);

    item->second.dwRefineComposeID = m->second.getTableInt("RefineEffectCost");
    item->second.dwCardSlot = m->second.getTableInt("CardSlot");
    item->second.dwSubID = m->second.getTableInt("SubstituteID");
    item->second.dwUpgID = m->second.getTableInt("UpgradeID");
    item->second.dwVID = m->second.getTableInt("VID");
    //add vid record
    {
      DWORD basevid = getBaseVID(item->second.dwVID);
      auto itv = m_mapBaseVID2Equips.find(basevid);
      if (itv == m_mapBaseVID2Equips.end())
      {
        vector<pair<DWORD, DWORD>>& equips = m_mapBaseVID2Equips[basevid];
        equips.push_back(std::make_pair(m->first, item->second.dwVID));
      }
      else
      {
        itv->second.push_back(std::make_pair(m->first, item->second.dwVID));
      }
    }

    item->second.dwMaxRefineLv = m->second.getTableInt("RefineMaxlv");
    item->second.dwDecomposeID = m->second.getTableInt("DecomposeID");
    if (item->second.dwDecomposeID != 0)
      m_mapDecomposeEquipCFG[item->second.dwDecomposeID].insert(m->first);
    item->second.dwDecomposeNum = m->second.getTableInt("DecomposeNum");
    item->second.dwForbid = m->second.getTableInt("ForbidFuncBit");

    // can equip
    xLuaData& profession = m->second.getMutableData("CanEquip");
    auto professionf = [&](const string& key, xLuaData& data)
    {
      DWORD profession = data.getInt();
      if (profession == 0)
        return;
      if (profession <= EPROFESSION_MIN || profession >= EPROFESSION_MAX || EProfession_IsValid(profession) == false)
      {
        bCorrect = false;
        XERR << "[物品配置-装备] id : " << item->first << " 穿戴职业 " << profession << " 不是合法的职业" << XEND;
        return;
      }
      item->second.vecEquipPro.push_back(static_cast<EProfession>(profession));
    };
    profession.foreach(professionf);

    // attr
    xLuaData& base = m->second.getMutableData("Effect");
    xLuaData& strength = m->second.getMutableData("EffectAdd");
    xLuaData& refine = m->second.getMutableData("RefineEffect");
    xLuaData& refine2 = m->second.getMutableData("RefineEffect2");
    xLuaData& pvpbase = m->second.getMutableData("PVPEffect");

    DWORD type = 0;
    auto collectAttr = [&](std::string key, xLuaData &data)
    {
      DWORD dwAttr = RoleDataConfig::getMe().getIDByName(key.c_str());
      if (dwAttr <= EATTRTYPE_MIN || dwAttr >= EATTRTYPE_MAX || EAttrType_IsValid(dwAttr) == false)
      {
        bCorrect = false;
        XERR << "[物品配置-装备] id : " << item->first << " attr : " << key.c_str() << " 未在 Table_RoleData.txt 表中找到" << XEND;
        return;
      }

      UserAttrSvr oAttr;
      oAttr.set_type(static_cast<EAttrType>(dwAttr));
      oAttr.set_value(data.getFloat());
      if (type == 0)
        item->second.vecEquipBase.push_back(oAttr);
      else if (type == 1)
        item->second.vecEquipStrength.push_back(oAttr);
      else if (type == 2)
        item->second.vecEquipRefine.push_back(oAttr);
      else if (type == 3)
        item->second.vecPVPEquipBase.push_back(oAttr);
    };
    type = 0, base.foreach(collectAttr);
    type = 1, strength.foreach(collectAttr);
    type = 2, refine.foreach(collectAttr);
    type = 2, refine2.foreach(collectAttr);
    type = 3, pvpbase.foreach(collectAttr);

    // buff
    xLuaData& buff = m->second.getMutableData("UniqueEffect").getMutableData("buff");
    auto buffF = [&item](const std::string& key, xLuaData& data)
    {
      item->second.vecBuffIDs.push_back(data.getInt());
    };
    buff.foreach(buffF);

    xLuaData& refinebuff = m->second.getMutableData("RefineBuff");
    auto refineBuffF = [&item](const std::string& key, xLuaData& data)
    {
      if (data.has("lv") == false)
        return;
      DWORD refine = data.getTableInt("lv");
      std::pair<DWORD, TVecDWORD> pa;
      pa.first = refine;
      auto buffSelfF = [&](const std::string& key, xLuaData data) 
      {
        pa.second.push_back(data.getInt());
      };

      xLuaData& xSelfBuff = data.getMutableData("buff");
      xSelfBuff.foreach(buffSelfF);

      item->second.vecRefine2Buff.push_back(pa);
    };
    refinebuff.foreach(refineBuffF);

    // suit refine attr
    // format: "suitid = {1,2,3}, Str = { lv=1, base=1, max=10}, Vit = { lv=1, base=1, max=10}"
    xLuaData& suitrefineattr = m->second.getMutableData("SuitRefineAttr");
    auto suitrefineattrF = [&](const std::string& key, xLuaData& data) {
      if (key == "suitid") {
        auto idsF = [&](const std::string& key, xLuaData& data) {
          item->second.vecSuitRefineAttrIDs.push_back(data.getInt());
        };
        data.foreach(idsF);
      } else {
        DWORD dwAttr = RoleDataConfig::getMe().getIDByName(key.c_str());
        if (dwAttr <= EATTRTYPE_MIN || dwAttr >= EATTRTYPE_MAX || EAttrType_IsValid(dwAttr) == false) {
          bCorrect = false;
          XERR << "[物品配置-装备] id : " << item->first << " suit refine attr : " << key.c_str() << " 未在 Table_RoleData.txt 表中找到" << XEND;
          return;
        }

        if (!data.has("lv") || !data.has("base") || !data.has("max")) {
          bCorrect = false;
          XERR << "[物品配置-装备] id : " << item->first << " suit refine attr : " << key.c_str() << " 缺少lv/base/max中某一字段" << XEND;
          return;
        }

        SEquipSuitRefineAttrCFG& rCfg = item->second.mapSuitRefineAttrs[dwAttr];
        rCfg.dwLevel = data.getTableInt("lv");
        rCfg.fBase = data.getTableFloat("base");
        rCfg.fMax = data.getTableFloat("max");
      }
    };
    suitrefineattr.foreach(suitrefineattrF);

    // sex equip
    DWORD sexEquip = m->second.getTableInt("SexEquip");
    if (sexEquip < EGENDER_MIN || sexEquip >= EGENDER_MAX || EGender_IsValid(sexEquip) == false) { // 0表示所有性别通用
      XERR << "[物品配置] id :" << m->first << "SexEquip :" << sexEquip << "不合法" << XEND;
      bCorrect = false;
      continue;
    }
    item->second.eSexEquip = static_cast<EGender>(sexEquip);
    item->second.dwBody = m->second.getTableInt("Body");
    item->second.dwGroupID = m->second.getTableInt("GroupID");

    // pvpbuff
    auto pvpbuffF = [&item](const std::string& key, xLuaData& data)
    {
      item->second.vecPVPBuffIDs.push_back(data.getInt());
    };
    m->second.getMutableData("PVPUniqueEffect").getMutableData("buff").foreach(pvpbuffF);

    item->second.setFashionBuffIDs.clear();
    m->second.getMutableData("FashionBuff").getIDList(item->second.setFashionBuffIDs);
  }

  if (bCorrect)
    XLOG << "[物品配置-装备] 成功加载配置Table_Equip.txt" << XEND;
  return bCorrect;
}

bool ItemConfig::loadEquipLotteryConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_EquipLottery.txt"))
  {
    XERR << "[物品配置-装备] 加载配置Table_EquipLottery.txt失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_EquipLottery", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    // get item config
    auto item = m_mapItemCFG.find(m->second.getTableInt("ItemId"));
    if (item == m_mapItemCFG.end())
    {
      XERR << "[物品配置-装备] id : " << m->first <<"itemid" << m->second.getTableInt("ItemId") << " 未在Table_Item.txt表中找到" << XEND;
      bCorrect = false;
      continue;
    }

    // init data    
    item->second.bLottery = true;
    LevelData lvData;
    xLuaData& rLv = m->second.getMutableData("Level");
    lvData.prLvRange.first = rLv.getTableInt("1");
    lvData.prLvRange.second = rLv.getTableInt("2");

    // attr
    xLuaData& base = m->second.getMutableData("Attr");
    auto collectAttr = [&](std::string key, xLuaData &data)
    {
      DWORD dwAttr = RoleDataConfig::getMe().getIDByName(key.c_str());
      if (dwAttr <= EATTRTYPE_MIN || dwAttr >= EATTRTYPE_MAX || EAttrType_IsValid(dwAttr) == false)
      {
        bCorrect = false;
        XERR << "[物品配置-装备] id : " << item->first << " attr : " << key.c_str() << " 未在 Table_RoleData.txt 表中找到" << XEND;
        return;
      }

      UserAttrSvr oAttr;
      oAttr.set_type(static_cast<EAttrType>(dwAttr));
      oAttr.set_value(data.getFloat());
      lvData.vecEquipBase.push_back(oAttr);
    };
    base.foreach(collectAttr);  
    item->second.vecLevelData.push_back(lvData);
  }

  if (bCorrect)
    XLOG << "[物品配置-装备] 成功加载配置Table_EquipLottery.txt" << XEND;
  return bCorrect;
}

bool ItemConfig::loadCardConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Card.txt"))
  {
    XERR << "[物品配置-卡片] 加载配置Table_Card.txt失败" << XEND;
    return false;
  }

  m_mapQualityCard.clear();
  m_dwCardCFGLoadTick = DWORD_MAX;

  DWORD dwNow = now();
  DWORD dwBranch = BaseConfig::getMe().getInnerBranch();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Card", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    // get config data
    auto item = m_mapItemCFG.find(m->first);
    if (item == m_mapItemCFG.end())
    {
      XERR << "[物品配置-卡片] id : " << m->first << " 未在 Table_Item.txt 表中找到" << XEND;
      bCorrect = false;
      continue;
    }

    // check item type
    if (item->second.eItemType < EITEMTYPE_CARD_WEAPON || item->second.eItemType > EITEMTYPE_CARD_HEAD)
    {
      XERR << "[物品配置-卡片] id : " << m->first << " type : " << item->second.eItemType << " 不是卡片类型" << XEND;
      bCorrect = false;
      continue;
    }

    // card time
    const string& lotteryparam = dwBranch == BRANCH_TF ? "TFLotteryDate" : "LotteryDate";
    xLuaData& lotterytime = m->second.getMutableData(lotteryparam.c_str());
    const string& ls = lotterytime.getTableString("1");
    if (ls.empty() == false)
      parseTime(ls.c_str(), item->second.dwCardLotteryStartTime);
    const string& le = lotterytime.getTableString("2");
    if (le.empty() == false)
      parseTime(le.c_str(), item->second.dwCardLotteryEndTime);

    const string& composeparam = dwBranch == BRANCH_TF ? "TFComposeDate" : "ComposeDate";
    xLuaData& composetime = m->second.getMutableData(composeparam.c_str());
    const string& cs = composetime.getTableString("1");
    if (cs.empty() == false)
      parseTime(cs.c_str(), item->second.dwCardComposeStartTime);
    const string& ce = composetime.getTableString("2");
    if (ce.empty() == false)
      parseTime(ce.c_str(), item->second.dwCardComposeEndTime);

    if (item->second.dwCardLotteryStartTime > dwNow && item->second.dwCardLotteryStartTime < m_dwCardCFGLoadTick)
      m_dwCardCFGLoadTick = item->second.dwCardLotteryStartTime;
    if (item->second.dwCardLotteryEndTime > dwNow && item->second.dwCardLotteryEndTime < m_dwCardCFGLoadTick)
      m_dwCardCFGLoadTick = item->second.dwCardLotteryEndTime;
    if (item->second.dwCardComposeStartTime > dwNow && item->second.dwCardComposeStartTime < m_dwCardCFGLoadTick)
      m_dwCardCFGLoadTick = item->second.dwCardComposeStartTime;
    if (item->second.dwCardComposeEndTime > dwNow && item->second.dwCardComposeEndTime < m_dwCardCFGLoadTick)
      m_dwCardCFGLoadTick = item->second.dwCardComposeEndTime;

    // card equip type
    item->second.dwCardPosition = m->second.getTableInt("Position");
    // card type
    item->second.dwCardType = m->second.getTableInt("CardType");
    // card lv
    item->second.dwLevel = m->second.getTableInt("CardLv");
    // card weight
    item->second.dwCardWeight = m->second.getTableInt("Weight");
    // card func
    item->second.dwCardFunc = m->second.getTableInt("Type");

    // attrs
    xLuaData& effect = m->second.getMutableData("Effect");
    auto attr = [&item, &bCorrect](const std::string& key, xLuaData& data)
    {
      DWORD type = RoleDataConfig::getMe().getIDByName(data.getTableString("name"));
      if (type <= EATTRTYPE_MIN || type >= EATTRTYPE_MAX || EAttrType_IsValid(type) == false)
      {
        bCorrect = false;
        XERR << "[物品配置-卡片] id : " << item->first << " attr : " << key.c_str() << " 未在 Table_RoleData.txt 表中找到" << XEND;
        return;
      }

      UserAttrSvr attr;
      attr.set_type(static_cast<EAttrType>(type));
      attr.set_value(data.getTableInt("value"));

      auto v = find_if(item->second.vecCardBase.begin(), item->second.vecCardBase.end(), [type](const UserAttrSvr& r) -> bool{
        return r.type() == type;
      });

      if (v == item->second.vecCardBase.end())
        item->second.vecCardBase.push_back(attr);
      else
        v->set_value(v->value() + data.getTableInt("value"));
    };
    effect.foreach(attr);

    //card buff
    xLuaData& buffData = m->second.getMutableData("BuffEffect").getMutableData("buff");
    auto buffF = [&](const std::string& key, xLuaData& data)
    {
      item->second.vecBuffIDs.push_back(data.getInt());
    };
    buffData.foreach(buffF);

    if (item->second.isTimeValid() == false)
    {
      const SItemCFG& rCFG = item->second;
      XLOG << "[物品配置-卡片] id :" << m->first << "当前服务器分支" << dwBranch << "该物品有效期在" << rCFG.dwCardLotteryStartTime << rCFG.dwCardLotteryEndTime << "之内,当前时间为" << dwNow << "该卡片被过滤" << XEND;
      continue;
    }

    // linker card quality
    m_mapQualityCard[item->second.eQualityType].vecCardCFG.push_back(item->second);

    // linker cardid and monsterid
    if (m->second.getTableInt("CardLv") != 1)
      continue;

    m_vecMonster2Card.clear();
    DWORD cardid = m->first;
    auto monsterf = [&](const string& str, xLuaData& data)
    {
      DWORD monsterID = data.getInt();
      if (getCardIDByMonsterID(monsterID) != 0)
        return;

      TPairMonsterCard p;
      p.first = monsterID;
      p.second = cardid;
      m_vecMonster2Card.push_back(p);
    };
    xLuaData& monster = m->second.getMutableData("monsterID");
    monster.foreach(monsterf);
  }

  // calc card weight
  for (auto &m : m_mapQualityCard)
  {
    for (auto &v : m.second.vecCardCFG)
    {
      v.dwCardWeight += m.second.dwMaxWeight;
      m.second.dwMaxWeight = v.dwCardWeight;
    }
    XLOG << "[物品配置-卡片] quality :" << m.first << "maxweight :" << m.second.dwMaxWeight << XEND;
  }

  if (bCorrect)
    XLOG << "[物品配置-卡片] 成功加载配置Table_Card.txt" << XEND;
  return bCorrect;
}

bool ItemConfig::loadMountConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Mount.txt"))
  {
    XERR << "[物品配置-坐骑] 加载配置Table_Mount.txt失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Mount", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    // get config data
    auto item = m_mapItemCFG.find(m->first);
    if (item == m_mapItemCFG.end())
    {
      XERR << "[物品配置-坐骑] id : " << m->first << " 未在 Table_Item.txt 表中找到" << XEND;
      bCorrect = false;
      continue;
    }

    // init data
    if (item->second.eItemType != EITEMTYPE_MOUNT)
    {
      XERR << "[物品配置-坐骑] id : " << m->first << " type : " << item->second.eItemType << " 不是坐骑类型" << XEND;
      bCorrect = false;
      continue;
    }
    item->second.eEquipType = EEQUIPTYPE_MOUNT;
  }

  if (bCorrect)
    XLOG << "[物品配置-坐骑] 成功加载配置Table_Mount.txt" << XEND;
  return bCorrect;
}

bool ItemConfig::loadMasterConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_EquipMaster.txt"))
  {
    XERR << "[物品配置-大师],加载配置Table_EquipMaster.txt失败" << XEND;
    return false;
  }

  m_vecMasterCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_EquipMaster", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SMasterCFG stCFG;

    const string& type = m->second.getTableString("type");
    if (type == "StrengthenMaster")
      stCFG.eType = EMASTERTYPE_STRENGTH;
    else if (type == "RefineMaster")
      stCFG.eType = EMASTERTYPE_REFINE;
    else
    {
      bCorrect = false;
      XERR << "[物品配置-大师] id : " << m->first << " type : " << type.c_str() << " 不合法类型" << XEND;
      continue;
    }

    stCFG.dwLv = m->second.getTableInt("Needlv");

    xLuaData& data = m->second.getMutableData("AddAttr");
    auto index = [&](const string& key, xLuaData& d)
    {
      DWORD type = RoleDataConfig::getMe().getIDByName(d.getTableString("name"));
      if (type <= EATTRTYPE_MIN || type >= EATTRTYPE_MAX || EAttrType_IsValid(type) == false)
      {
        bCorrect = false;
        XERR << "[物品配置-大师] id : " << m->first << " attr : " << key.c_str() << " 未在 Table_RoleData.txt 表中找到" << XEND;
        return;
      }

      UserAttrSvr oAttr;
      oAttr.set_type(static_cast<EAttrType>(type));
      oAttr.set_value(d.getTableInt("value"));

      stCFG.vecAttrs.push_back(oAttr);
    };
    data.foreach(index);

    m_vecMasterCFG.push_back(stCFG);
  }

  if (bCorrect)
    XLOG << "[物品配置-大师] 成功加载配置Table_EquipMaster.txt" << XEND;
  return bCorrect;
}

bool ItemConfig::loadSuitConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_EquipSuit.txt"))
  {
    XERR << "[物品配置-大师],加载配置Table_EquipSuit.txt失败" << XEND;
    return false;
  }

  m_mapSuitCFG.clear();
  m_mapEquipID2SuitID.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_EquipSuit", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SSuitCFG stCFG;

    // id
    stCFG.id = m->first;

    // equip id
    xLuaData& equip = m->second.getMutableData("Suitid");
    auto equipf = [this, &stCFG](const string& key, xLuaData& d)
    {
      stCFG.vecEquipIDs.push_back(d.getInt());
      auto it = m_mapEquipID2SuitID.find(d.getInt());
      if (it != m_mapEquipID2SuitID.end())
        it->second.insert(stCFG.id);
      else
        m_mapEquipID2SuitID[d.getInt()].insert(stCFG.id);
    };
    equip.foreach(equipf);

    // attrs
    const string name[] = {"SuitOneAdd", "SuitTwoAdd", "SuitThreeAdd", "SuitFourAdd", "SuitFiveAdd", "SuitSixAdd"};
    DWORD count = 1;
    for (string s : name)
    {
      xLuaData& buff = m->second.getMutableData(s.c_str()).getMutableData("buff");

      TPairCountBuff cntbuff;
      cntbuff.first = count;
      auto funbuff = [this, &cntbuff](const string& key, xLuaData& d)
      {
        cntbuff.second.push_back(d.getInt());
      };

      buff.foreach(funbuff);
      if (cntbuff.second.empty() == false)
      stCFG.vecBuffs.push_back(cntbuff);

      ++count;
    }

    // refine buff
    /* format: RefineBuff = { lv=x, buff={x,x,x}, equips={ {id=x, lv=x, buff={x,x}}, {id=x, lv=x, buff={x,x}} } */
    if (m->second.has("RefineBuff")) {
        xLuaData& refinebuff = m->second.getMutableData("RefineBuff");
        if (refinebuff.has("lv")) {
          stCFG.dwRefineLevel = refinebuff.getTableInt("lv");
        }
        if (refinebuff.has("buff")) {
          xLuaData& buff = refinebuff.getMutableData("buff");
          auto funcbuff = [&stCFG](const string& key, xLuaData& d) {
            stCFG.vecRefineBuffs.push_back(d.getInt());
          };
          buff.foreach(funcbuff);
        }
        if (refinebuff.has("equips")) {
          xLuaData& equips = refinebuff.getMutableData("equips");
          auto funcequips = [&stCFG](const string& key, xLuaData& d) {
            SEquipRefineBuff rb;
            if (d.has("lv")) {
              rb.dwLevel = d.getTableInt("lv");
            } else {
              return;
            }
            if (d.has("buff")) {
              xLuaData& buff = d.getMutableData("buff");
              auto funcbuff = [&rb](const string& key, xLuaData& d) {
                rb.vecBuffs.push_back(d.getInt());
              };
              buff.foreach(funcbuff);
            } else {
              return;
            }
            if (d.has("id")) {
              stCFG.mapEquipRefineBuffs[d.getTableInt("id")] = rb;
            } else {
              return;
            }
          };
          equips.foreach(funcequips);
        }
      }
       
    // insert to list
    m_mapSuitCFG[m->first] = stCFG;
  }

  // check equip id valid
  for (auto m = m_mapEquipID2SuitID.begin(); m != m_mapEquipID2SuitID.end(); ++m)
  {
    const SItemCFG* pCFG = getItemCFG(m->first);
    if (pCFG == nullptr || (isFashion(pCFG->eItemType) == false && isEquip(pCFG->eItemType) == false && isCard(pCFG->eItemType) == false))
    {
      for (auto &v : m->second)
      {
        const SSuitCFG* pSuitCFG = getSuitCFG(v);
        XERR << "[物品配置-套装] id : " << m->first << " suitid : " << pSuitCFG->id << " 未在 Table_Equip.txt 或 Table_Card.txt 表中找到" << XEND;
        bCorrect = false;
        continue;
      }
    }
  }

  if (bCorrect)
    XLOG << "[物品配置-套装] 成功加载配置Table_EquipSuit.txt" << XEND;
  return bCorrect;
}

bool ItemConfig::loadRefineConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_EquipRefine.txt"))
  {
    XERR << "[物品配置-精炼],加载配置Table_EquipRefine.txt失败" << XEND;
    return false;
  }

  m_vecRefineCFG.clear();
  m_vecRefineTradeCFG.clear();

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_EquipRefine", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    // equiptype
    vector<EItemType> vecTypes;
    bool bSuccess = true;
    xLuaData& equiptype = m->second.getMutableData("EuqipType");
    auto type = [&vecTypes, &bSuccess, m](const string& key, xLuaData& data)
    {
      DWORD itemtype = data.getInt();
      if (itemtype <= EITEMTYPE_MIN || itemtype >= EITEMTYPE_MAX || EItemType_IsValid(itemtype) == false)
      {
        bSuccess = false;
        XERR << "[物品配置-精炼] id : " << m->first << " equiptype : " << itemtype << " 不合法" << XEND;
        return;
      }

      EItemType eType = static_cast<EItemType>(itemtype);
      vecTypes.push_back(eType);
    };
    equiptype.foreach(type);

    DWORD costConfigNum = 0, bingoConfigNum = 0, failConfigNum = 0, damageConfigNum = 0;
    vector<SRefineRateCFG> vecComposeRate;
    xLuaData& refineCost = m->second.getMutableData("RefineCost");
    auto cost = [&](const string& key, xLuaData& data)
    {
      SRefineRateCFG composeRate;
      DWORD quality = data.getTableInt("color");
      if (quality <= EQUALITYTYPE_MIN || quality >= EQUALITYTYPE_MAX)
      {
        bSuccess = false;
        XERR << "[物品配置-精炼], [RefineCost], id : " << m->first << " color : " << quality << " value invalid!" << XEND;
        return;
      }
      if (EQualityType_IsValid(quality) == false)
      {
        bSuccess = false;
        XERR << "[物品配置-精炼], [RefineCost], id : " << m->first << " color : " << quality << " value invalid!" << XEND;
        return;
      }
      EQualityType eQuality = static_cast<EQualityType>(quality);
      composeRate.eQuaType = eQuality;

      auto compose = [&composeRate] (const string& key, xLuaData& data)
      {
        composeRate.vecCompose.push_back(data.getInt());
      };
      data.getMutableData("id").foreach(compose);
      vecComposeRate.push_back(composeRate);
      costConfigNum ++;
    };
    refineCost.foreach(cost);

    xLuaData& safeRefineCost = m->second.getMutableData("SafeRefineCost");
    auto safecost = [&](const string& key, xLuaData& data)
    {
      DWORD quality = data.getTableInt("color");
      if (quality <= EQUALITYTYPE_MIN || quality >= EQUALITYTYPE_MAX)
      {
        bSuccess = false;
        XERR << "[物品配置-精炼], [SafeRefineCost], id : " << m->first << " color : " << quality << " value invalid!" << XEND;
        return;
      }
      if (EQualityType_IsValid(quality) == false)
      {
        bSuccess = false;
        XERR << "[物品配置-精炼], [SafeRefineCost], id : " << m->first << " color : " << quality << " value invalid!" << XEND;
        return;
      }
      EQualityType eQuality = static_cast<EQualityType>(quality);
      auto v = find_if(vecComposeRate.begin(), vecComposeRate.end(), [eQuality](const SRefineRateCFG& r) -> bool{
        return r.eQuaType == eQuality;
      });
      if (v == vecComposeRate.end())
      {
        bSuccess = false;
        XERR << "[物品配置-安全精炼][SafeRefineCost] id :" << m->first << "color :" << eQuality << "未找到该品质的精炼配置" << XEND;
        return;
      }
      //composeRate.eQuaType = eQuality;

      auto compose = [&v] (const string& key, xLuaData& data)
      {
        v->vecSafeCompose.push_back(data.getInt());
      };
      data.getMutableData("id").foreach(compose);
    };
    safeRefineCost.foreach(safecost);

    xLuaData& refinereturn = m->second.getMutableData("RefineZenyReturn");
    auto refineret = [&](const string& key, xLuaData& data)
    {
      DWORD quality = data.getTableInt("color");
      if (quality <= EQUALITYTYPE_MIN || quality >= EQUALITYTYPE_MAX || EQualityType_IsValid(quality) == false)
      {
        bSuccess = false;
        XERR << "[物品配置-精炼][RefineZenyReturn] id :" << m->first << "color :" << quality << "value invalid!" << XEND;
        return;
      }
      EQualityType eQuality = static_cast<EQualityType>(quality);
      auto v = find_if(vecComposeRate.begin(), vecComposeRate.end(), [eQuality](const SRefineRateCFG& r) -> bool{
        return r.eQuaType == eQuality;
      });
      if (v == vecComposeRate.end())
      {
        bSuccess = false;
        XERR << "[物品配置-精炼][RefineZenyReturn] id :" << m->first << "color :" << quality << "未找到该品质的精炼配置" << XEND;
        return;
      }
      v->dwReturnZeny = data.getTableInt("num");
    };
    refinereturn.foreach(refineret);

    auto checkQuaMatch = [&vecComposeRate] (DWORD quality, DWORD rate, DWORD ratetype) -> bool
    {
      if (quality <= EQUALITYTYPE_MIN || quality >= EQUALITYTYPE_MAX)
        return false;
      if (EQualityType_IsValid(quality) == false)
        return false;
      EQualityType eType = static_cast<EQualityType>(quality);
      for (auto v = vecComposeRate.begin(); v != vecComposeRate.end(); ++v)
      {
        if (eType == v->eQuaType)
        {
          if (ratetype == 1)
            v->dwSuccessRate = rate;
          else if (ratetype == 2)
            v->dwFailStayRate = rate;
          else if (ratetype == 3)
            v->dwFailNoDamageRate = rate;
          return true;
        }
      }
      return false;
    };

    xLuaData& refineSucc = m->second.getMutableData("BingoRate");
    auto succRate = [&] (const string& key, xLuaData& data)
    {
      DWORD quality = data.getTableInt("color");
      DWORD rate = data.getTableInt("rate");
      if (checkQuaMatch(quality, rate, 1) == false)
      {
        bSuccess = false;
        XERR << "[物品配置-精炼], [BingoRate], id : " << m->first << " color : " << quality << " value invalid!" << XEND;
        return;
      }
      bingoConfigNum ++;
    };
    refineSucc.foreach(succRate);

    xLuaData& refineFailStay = m->second.getMutableData("FailRate");
    auto failStayRate = [&] (const string& key, xLuaData& data)
    {
      DWORD quality = data.getTableInt("color");
      DWORD rate = data.getTableInt("rate");
      if (checkQuaMatch(quality, rate, 2) == false)
      {
        bSuccess = false;
        XERR << "[物品配置-精炼], [FailRate], id : " << m->first << " color : " << quality << " value invalid!" << XEND;
        return;
      }
      failConfigNum ++;
    };
    refineFailStay.foreach(failStayRate);

    xLuaData& refineDamage = m->second.getMutableData("DamageRate");
    auto damageRate = [&] (const string& key, xLuaData& data)
    {
      DWORD quality = data.getTableInt("color");
      DWORD rate = data.getTableInt("rate");
      if (checkQuaMatch(quality, rate, 3) == false)
      {
        bSuccess = false;
        XERR << "[物品配置-精炼], [DamageRate], id : " << m->first << " color : " << quality << " value invalid!" << XEND;
        return;
      }
      damageConfigNum ++;
    };
    refineDamage.foreach(damageRate);

    if (!bSuccess)
    {
      bCorrect = false;
      continue;
    }

    if (costConfigNum != bingoConfigNum || costConfigNum != failConfigNum || costConfigNum != damageConfigNum)
    {
      XERR << "[物品配置-精炼], 品质配置不匹配, id : " << m->first << XEND;
      bCorrect = false;
      continue;
    }

    DWORD refinelv = m->second.getTableInt("RefineLv");

    DWORD dwEquipRate = m->second.getTableInt("EquipRate");
    DWORD dwEquipRateNew = m->second.getTableInt("EquipRate_1");
    DWORD dwItemID = m->second.getTableInt("ItemID");
    DWORD dwItemRate = m->second.getTableInt("ItemRate");
    DWORD dwItemRateNew = m->second.getTableInt("ItemRate_1");
    DWORD dwLvRate = m->second.getTableInt("LvRate");
    DWORD dwLvRateNew = m->second.getTableInt("LvRate_1");
    float fRiskRate = m->second.getTableFloat("RiskRate");

    // merge equiptype, refinelv, cost and rate to config
    for (auto v = vecTypes.begin(); v != vecTypes.end(); ++v)
    {
      for (auto c = vecComposeRate.begin(); c != vecComposeRate.end(); ++c)
      {
        if (getRefineCFG(*v, (*c).eQuaType, refinelv) != nullptr)
        {
          bCorrect = false;
          XERR << "[物品配置-精炼], 精炼配置重复, type:" << *v << ", quality:" << (*c).eQuaType << ", lv:" << refinelv << XEND;
          continue;
        }
        SRefineCFG oneCFG;
        oneCFG.eItemType = *v;
        oneCFG.sComposeRate = *c;
        oneCFG.dwRefineLv = refinelv;
        m_vecRefineCFG.push_back(oneCFG);
      }
      SRefineTradeCFG tradeCFG;
      tradeCFG.eItemType = *v;
      tradeCFG.dwRefineLv = refinelv;
      tradeCFG.dwEquipRate = dwEquipRate;
      tradeCFG.dwEquipRateNew = dwEquipRateNew;      
      tradeCFG.dwItemID = dwItemID;
      tradeCFG.dwItemRate = dwItemRate;
      tradeCFG.dwItemRateNew = dwItemRateNew;
      tradeCFG.dwLvRate = dwLvRate;
      tradeCFG.dwLvRateNew = dwLvRateNew;
      tradeCFG.fRiskRate = fRiskRate;
      m_vecRefineTradeCFG.push_back(tradeCFG);
    }
  }

  if (bCorrect)
    XLOG << "[RefineConfig] 成功加载配置Table_EquipRefine.txt" << XEND;
  return bCorrect;
}

// 2016-04-19 申林移除Table_EquipFashion.txt表
/*bool ItemConfig::loadFashionConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_EquipFashion.txt"))
  {
    XERR("[Table_EquipFashion], 加载配置失败");
    return false;
  }
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_EquipFashion", table);

  m_vecFashionCFG.clear();
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SFashionCFG sfashion;
    sfashion.id = m->first;
    sfashion.num = m->second.getTableInt("EquipNumber");
    xLuaData& attrd = m->second.getMutableData("AttrAdd");
    auto funattr = [this, &sfashion](const string& key, xLuaData& d)
    {
      DWORD dwAttr = RoleDataConfig::getMe().getIDByName(key.c_str());
      if (dwAttr <= EATTRTYPE_MIN || dwAttr >= EATTRTYPE_MAX || EAttrType_IsValid(dwAttr) == false)
      {
        XERR("[EquipFashion] id = %u attr = %s invalid!", sfashion.id, key.c_str());
        return;
      }
      UserAttrSvr oAttr;
      oAttr.set_type(static_cast<EAttrType>(dwAttr));
      oAttr.set_value(d.getFloat());
      sfashion.uAttrs.push_back(oAttr);
    };
    attrd.foreach(funattr);

    m_vecFashionCFG.push_back(sfashion);
  }

  if (bCorrect)
    XLOG("[EquipFashion], 加载配置Table_EquipFashion.txt");
  return bCorrect;
}*/

bool ItemConfig::loadItemOriginConfig()
{
  bool bCorrect = true;
  /*if (!xLuaTable::getMe().open("Lua/Table_ItemOrigin.txt"))
  {
    XERR("[Table_ItemOrigin], 加载配置失败");
    return false;
  }
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_ItemOrigin", table);

  TSetItemNoSource setItems;
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    xLuaData& param1 = m->second.getMutableData("1");
    DWORD dwParam = param1.getTableInt("1");
    if (dwParam == 1000)
      continue;

    setItems.insert(m->first);
  }

  for (auto s = setItems.begin(); s != setItems.end(); ++s)
  {
    const SItemCFG* pCFG = getItemCFG(*s);
    if (pCFG == nullptr)
    {
      XERR("[Table_ItemOrigin] id = %u not exist in Table_Item.txt", *s);
      bCorrect = false;
      continue;
    }
    if (canEnterManual(pCFG) == false)
      continue;

    m_setItemNoSource.insert(*s);
  }

  if (bCorrect)
    XLOG("[Table_ItemOrigin], 加载配置Table_ItemOrigin.txt");*/
  return bCorrect;
}

bool ItemConfig::loadItemType()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_ItemType.txt"))
  {
    XERR << "[Table_ItemType.txt], 加载配置失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_ItemType", table);
  m_mapType2Error.clear();
  m_setItemShow.clear();
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD dwType = m->first;
    if (dwType <= EITEMTYPE_MIN || dwType >= EITEMTYPE_MAX || EItemType_IsValid(dwType) == false)
      continue;
    EItemType eType = static_cast<EItemType>(dwType);

    if (m->second.getTableInt("EffectShow") != 0 || m->second.getTableInt("CardShow") != 0)
      m_setItemShow.insert(eType);

    auto n = m_mapType2Error.find(eType);
    if (n != m_mapType2Error.end())
    {
      XERR << "[ItemType] id : " << dwType << " duplicated" << XEND;
      bCorrect = false;
      continue;
    }

    SItemErrorCFG stCFG;
    stCFG.eType = eType;
    stCFG.name = m->second.getTableString("Name");
    stCFG.dwUseNumber = m->second.getTableInt("UseNumber");
    xLuaData& msg = m->second.getMutableData("Sysmsg");
    stCFG.dwLvErrMsg = msg.getTableInt("levelsys");

    m_mapType2Error[stCFG.eType] = stCFG;
  }

  return bCorrect;
}

bool ItemConfig::loadEnchantConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_EquipEnchant.txt"))
  {
    XERR << "[Table_EquipEnchant.txt], 加载配置失败" << XEND;
    return false;
  }

  for (int i = EENCHANTTYPE_MIN + 1; i < EENCHANTTYPE_MAX; ++i)
    m_arrEnchantCFG[i] = SEnchantCFG();

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_EquipEnchant", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD dwType = m->second.getTableInt("EnchantType");
    if (dwType <= EENCHANTTYPE_MIN || dwType >= EENCHANTTYPE_MAX || EEnchantType_IsValid(dwType) == false)
    {
      XERR << "[附魔配置-加载] id : " << m->first << " EnchantType : " << dwType << " 不合法" << XEND;
      bCorrect = false;
      continue;
    }

    SEnchantCFG& rCFG = m_arrEnchantCFG[dwType];
    rCFG.eType = static_cast<EEnchantType>(dwType);
    rCFG.dwMaxNum = m->second.getTableInt("MaxNum");

    xLuaData& itemcost = m->second.getMutableData("ItemCost");
    rCFG.oNeedItem.set_id(itemcost.getTableInt("itemid"));
    rCFG.oNeedItem.set_count(itemcost.getTableInt("num"));

    rCFG.dwRob = m->second.getTableInt("ZenyCost");

    DWORD dwAttr = RoleDataConfig::getMe().getIDByName(m->second.getTableString("AttrType"));
    if (dwAttr == 0)
    {
      XERR << "[附魔配置-加载] id : " << m->first << " AttrType : " << m->second.getTableString("AttrType") << " 不合法" << XEND;
      bCorrect = false;
      continue;
    }

    /*if (rCFG.getEnchantAttr(static_cast<EAttrType>(dwAttr)) != nullptr)
    {
      XERR << "[附魔配置-加载] id : " << m->first << " AttrType : " << m->second.getTableString("AttrType") << " 重复了" << XEND;
      bCorrect = false;
      continue;
    }
    */

    SEnchantAttr stAttr;
    stAttr.dwConfigID = m->second.getTableInt("UniqID");
    stAttr.eType = static_cast<EAttrType>(dwAttr);

    xLuaData& attr2 = m->second.getMutableData("AttrType2");
    auto attr2f = [&](const string& str, xLuaData& data)
    {
      DWORD dwAttr2 = RoleDataConfig::getMe().getIDByName(data.getString());
      if (dwAttr2 != 0)
      {
        if (dwAttr2 <= EATTRTYPE_MIN || dwAttr2 >= EATTRTYPE_MAX || EAttrType_IsValid(dwAttr2) == false)
        {
          XERR << "[附魔配置-加载] id : " << m->first << " AttrType2 : " << data.getString() << " 不合法" << XEND;
          bCorrect = false;
          return;
        }
      }
      stAttr.vecPairType.push_back(static_cast<EAttrType>(dwAttr2));
    };
    attr2.foreach(attr2f);

    //是否算交易所极品附魔
    xLuaData& noGoodEnchant = m->second.getMutableData("NoExchangeEnchant");
    std::vector<EItemType> vecNoGoodEnchant;
    vecNoGoodEnchant.reserve(20);
    auto noGoodF = [&](const string& key, xLuaData& data)
    {
      EItemType eItemType = getItemType(key);
      vecNoGoodEnchant.push_back(eItemType);
    };
    noGoodEnchant.foreach(noGoodF);

    //额外buffid
    xLuaData& extrabuff = m->second.getMutableData("AddAttr");
    auto extrabufff = [&](const string& key, xLuaData& data)
    {
      pair<DWORD, DWORD> p;
      p.first = data.getTableInt("1");      //buffid 
      p.second = data.getTableInt("2");
      stAttr.vecExtraBuff.push_back(p);
      
      if (!vecNoGoodEnchant.empty())
        rCFG.mapNoGoodEnchant[p.first] = vecNoGoodEnchant;
    };
    extrabuff.foreach(extrabufff);

    xLuaData& condition = m->second.getMutableData("Condition");
    stAttr.vecExtraCondition.push_back(condition.getTableInt("type"));
    if (stAttr.vecExtraCondition[0] == EENCHANTEXTRACON_REFINELV)
      stAttr.vecExtraCondition.push_back(condition.getTableInt("refinelv"));

    stAttr.fMin = m->second.getMutableData("AttrBound").getMutableData("1").getTableFloat("1");
    stAttr.fMax = m->second.getMutableData("AttrBound").getMutableData("1").getTableFloat("2");
    stAttr.fExpressionOfMaxUp = m->second.getTableFloat("ExpressionOfMaxUp");

    float fLastValue = stAttr.fMin;
    xLuaData& rate = m->second.getMutableData("MaxAttrRate");
    auto ratef = [&](const string& str, xLuaData& data)
    {
      SEnchantAttrItem stItem;
      stItem.fMin = fLastValue;
      stItem.fMax = data.getTableFloat("1");
      stItem.dwWeight = data.getTableInt("2");
      stItem.dwRawWeight = stItem.dwWeight;

      if (stItem.fMin > stItem.fMax + 0.001f)
      {
        XERR << "[附魔配置-加载] id : " << m->first << " min : " << stItem.fMin << " 大于 max : " << stItem.fMax << XEND;
        bCorrect = false;
        return;
      }

      stAttr.vecItems.push_back(stItem);

      fLastValue = stItem.fMax + 0.001f;
    };
    rate.foreach(ratef);

    //NoShowEquip
    //是否能随出额外buffid
    xLuaData& noShowEquip = m->second.getMutableData("NoShowEquip");
    auto noShowF = [&](const string& key, xLuaData& data)
    {
      EItemType eItemType = getItemType(key);
      stAttr.setNoBuffid.insert(eItemType);
    };
    noShowEquip.foreach(noShowF);

    static const string equipname[] = { "SpearRate", "SwordRate", "StaffRate", "KatarRate", "BowRate", "MaceRate", "AxeRate", "BookRate", "KnifeRate", "InstrumentRate", "LashRate", "PotionRate",
                                        "GloveRate", "ArmorRate", "ShieldRate", "RobeRate", "ShoeRate", "AccessoryRate", "OrbRate", "EikonRate", "BracerRate", "BraceletRate", "TrolleyRate",
                                        "HeadRate", "FaceRate", "MouthRate", "TailRate", "WingRate"};

    for (auto &s : equipname)
    {
      if (m->second.getTableInt(s.c_str()) == 0)
        continue;
      EItemType eItemType = getItemType(s);
      if (eItemType == EITEMTYPE_MIN)
        continue;
      stAttr.setValidItemType.insert(eItemType);
    }

    rCFG.vecAttrs.push_back(stAttr);

    auto equip = [&](const string& str)
    {
      if (m->second.getTableInt(str.c_str()) == 0)
        return;

      EItemType eItemType = getItemType(str);
      if (eItemType == EITEMTYPE_MIN)
      {
        XERR << "[附魔配置-加载] id : " << m->first << " " << str.c_str() << " 类型不合法" << XEND;
        bCorrect = false;
        return;
      }

      const SEnchantEquip* pOriEquip = rCFG.getEnchantEquip(eItemType);
      if (pOriEquip)
      {
        for (auto &v : pOriEquip->vecItems)
        {
          if (v.p.first == dwAttr)
          {
            XERR << "[附魔配置-加载] id : " << m->first << " AttrType : " << m->second.getTableString("AttrType") << " 重复了" << XEND;
            bCorrect = false;
            return;
          }
        }
      }

      auto v = find_if(rCFG.vecEquips.begin(), rCFG.vecEquips.end(), [eItemType](const SEnchantEquip& r) -> bool{
        return r.eItemType == eItemType;
      });
      if (v == rCFG.vecEquips.end())
      {
        SEnchantEquip stEquip;
        stEquip.eItemType = eItemType;
        rCFG.vecEquips.push_back(stEquip);
        v = find_if(rCFG.vecEquips.begin(), rCFG.vecEquips.end(), [eItemType](const SEnchantEquip& r) -> bool{
          return r.eItemType == eItemType;
        });
      }
      if (v == rCFG.vecEquips.end())
      {
        XERR << "[附魔配置-加载] id : " << m->first << " 创建equip配置失败" << XEND;
        bCorrect = false;
        return;
      }

      SEnchantEquipItem stItem;
      stItem.p.first = static_cast<EAttrType>(dwAttr);
      stItem.p.second = m->second.getTableInt(str.c_str());
      v->vecItems.push_back(stItem);
    };

    for (auto &s : equipname)
      equip(s);   
  }

  for (int i = EENCHANTTYPE_MIN; i < EENCHANTTYPE_MAX; ++i)
  {
    SEnchantCFG& rCFG = m_arrEnchantCFG[i];
    for (auto &v : rCFG.vecAttrs)
    {
      for (auto &extra : v.vecExtraBuff)
      {
        extra.second += v.dwExtraMaxWeigth;
        v.dwExtraMaxWeigth = extra.second;
      }
      for (auto &item : v.vecItems)
      {
        item.dwWeight += v.dwMaxWeight;
        v.dwMaxWeight = item.dwWeight;
        //XDBG("[附魔配置-加载] entype : %u attrtype : %u min : %.2f max : %.2f weight : %u maxweight : %u", rCFG.eType, v.eType, item.fMin, item.fMax, item.dwWeight, v.dwMaxWeight);
      }
    }

    for (auto &equip : rCFG.vecEquips)
    {
      DWORD dwMaxWeight = 0;
      for (auto & item : equip.vecItems)
      {
        if (item.p.second == 0)
        {
          //XDBG("[附魔配置-加载] entype : %u itemtype : %u attrtype : %u weigth : %u maxweight : %u", rCFG.eType, equip.eItemType, item.p.first, item.p.second, item.dwMaxWeight);
          continue;
        }
        dwMaxWeight += item.p.second;        
      }      
      equip.dwMaxWeight = dwMaxWeight;
    }
  }

  if (bCorrect)
    XLOG << "[Table_EquipEnchant.txt], 成功加载配置" << XEND;
  return bCorrect;
}

// 2016-04-19 申林移除Table_EquipFashion.txt表
/*void ItemConfig::getFashionAttr(DWORD num, TVecAttrSvrs& vecAttrs)
{
  for (auto &v : m_vecFashionCFG)
  {
    if (v.num <= num)
    {
      for (auto &s : v.uAttrs)
      {
        float value = vecAttrs[s.type()].value() + s.value();
        vecAttrs[s.type()].set_value(value);
      }
    }
  }
}*/

bool ItemConfig::loadEnchantPriceConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_EquipEnchantPrice.txt"))
  {
    XERR << "[Table_EquipEnchantPrice.txt], 加载配置失败" << XEND;
    return false;
  }

  m_mapEnchantPriceCFG.clear();

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_EquipEnchantPrice", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD dwAttr = RoleDataConfig::getMe().getIDByName(m->second.getTableString("AttrType"));
    if (dwAttr == 0)
    {
      XERR << "[附魔价格配置-加载] id : " << m->first << " AttrType : " << m->second.getTableString("AttrType") << " 不合法" << XEND;
      bCorrect = false;
      continue;
    }
    
    auto it = m_mapEnchantPriceCFG.find(dwAttr);
    if (it != m_mapEnchantPriceCFG.end())
    {
      XERR << "[附魔价格配置-加载] id : " << m->first << " AttrType : " << m->second.getTableString("AttrType") << " 重复了" << XEND;
      bCorrect = false;
      continue;;
    }
    SEnchantPriceCFG& rCfg = m_mapEnchantPriceCFG[dwAttr];

    auto equip = [&](const string& str)
    {
      EItemType eItemType = getItemType(str);
      if (eItemType == EITEMTYPE_MIN)
      {
        XERR << "[附魔价格配置-加载] id : " << m->first << " " << str.c_str() << " 类型不合法" << XEND;
        bCorrect = false;
        return;
      }
      
      auto it = rCfg.mapValue.find(eItemType);
      if (it != rCfg.mapValue.end())
      {
        XERR << "[附魔价格配置-加载] id : " << m->first << " " << str.c_str() << " 重复的道具类型" << XEND;
        bCorrect = false;
        return;
      }      
      rCfg.mapValue[eItemType] = m->second.getTableFloat(str.c_str());
    };

    static const string equipname[] = { "SpearRate", "SwordRate", "StaffRate", "KatarRate", "BowRate", "MaceRate", "AxeRate", "BookRate", "KnifeRate", "InstrumentRate", "LashRate", "PotionRate",
      "GloveRate", "ArmorRate", "ShieldRate", "RobeRate", "ShoeRate", "AccessoryRate", "OrbRate", "EikonRate", "BracerRate", "BraceletRate", "TrolleyRate",
      "HeadRate", "FaceRate", "MouthRate", "TailRate", "WingRate"};
    for (auto &s : equipname)
      equip(s);
  }

  if (bCorrect)
    XLOG << "[Table_EquipEnchantPrice.txt], 成功加载配置" << XEND;
  return bCorrect;
}

bool ItemConfig::loadHairStyle()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_HairStyle.txt"))
  {
    XERR << "[物品配置-发型] 加载 Table_HairStyle.txt 表失败" << XEND;
    return false;
  }

  m_mapHairID2ItemID.clear();

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_HairStyle", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD dwItemID = m->second.getTableInt("ItemID");
    if (dwItemID == 0)
      continue;
    auto item = m_mapItemCFG.find(dwItemID);
    if (item == m_mapItemCFG.end())
    {
      XERR << "[物品配置-发型] hairid :" << m->first << "itemid :" << dwItemID << "未在Table_Item.txt表中找到" << XEND;
      bCorrect = false;
      continue;
    }

    item->second.dwHairID = m->first;
    m_mapHairID2ItemID[item->second.dwHairID] = dwItemID;
  }

  if (bCorrect)
    XLOG << "[物品配置-发型] 成功加载 Table_HairStyle.txt 表" << XEND;
  return bCorrect;
}

bool ItemConfig::loadAppellationConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Appellation.txt"))
  {
    XERR << "[AppellationConfig],加载配置Table_Appellation.txt失败" << XEND;
    return false;
  }

  std::map<DWORD, std::set<DWORD>> head;
  std::map<DWORD, std::set<DWORD>> tail;
  mapTitle2PreID.clear();

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Appellation", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    if (MiscConfig::getMe().isForbid("Appellation", m->first))
      continue;

    // get item config
    auto item = m_mapItemCFG.find(m->first);
    if (item == m_mapItemCFG.end())
    {
      XERR << "[物品配置-称号] id : " << m->first << " 未在Table_Item.txt表中找到" << XEND;
      bCorrect = false;
      continue;
    }

    DWORD type = m->second.getTableInt("GroupID");
    if (type <= ETITLE_TYPE_MIN || type >= ETITLE_TYPE_MAX)
    {
      XERR << "[TitleConfig] id = " << m->first << " type = " << type << " error!" << XEND;
      bCorrect = false;
      continue;
    }

    item->second.eTitleType = static_cast<ETitleType>(type);
    item->second.dwPostId = m->second.getTableInt("PostID");
    item->second.bFirst = false;
    if(item->second.dwPostId != 0)
      mapTitle2PreID.insert(std::make_pair(item->second.dwPostId, m->first));


    // attr
    xLuaData& base = m->second.getMutableData("BaseProp");

    auto collectAttr = [&](std::string key, xLuaData &data)
    {
      DWORD dwAttr = RoleDataConfig::getMe().getIDByName(key.c_str());
      if (dwAttr <= EATTRTYPE_MIN || dwAttr >= EATTRTYPE_MAX || EAttrType_IsValid(dwAttr) == false)
      {
        bCorrect = false;
        XERR << "[物品配置-称号] id : " << m->first << " attr : " << key.c_str() << " 未在 Table_RoleData.txt 表中找到" << XEND;
        return;
      }

      UserAttrSvr oAttr;
      oAttr.set_type(static_cast<EAttrType>(dwAttr));
      oAttr.set_value(data.getFloat());
      item->second.vecTitleBase.push_back(oAttr);
    };
    base.foreach(collectAttr);

    head[type].insert(m->first);
    tail[type].insert(item->second.dwPostId);
  }

  if (bCorrect)
  {
    XLOG << "[TitleConfig] 成功加载Table_Appellation.txt" << XEND;

    for (auto it = head.begin(); it != head.end(); ++it)
    {
      for(auto iter = it->second.begin(); iter != it->second.end(); ++iter)
      {
        auto tailit = tail.find(it->first);
        if(tailit == tail.end())
          continue;
        if(tailit->second.find(*iter) == tailit->second.end())
        {
          //is first
          auto item = m_mapItemCFG.find(*iter);
          if (item != m_mapItemCFG.end())
          {
            item->second.bFirst = true;
          }
        }
      }
    }
  }
  return bCorrect;
}

bool ItemConfig::loadTitleConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Title.txt"))
  {
    XERR << "[TitleConfig],加载配置Table_Title.txt失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Title", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    // get item config
    auto item = m_mapItemCFG.find(m->first);
    if (item == m_mapItemCFG.end())
    {
      XERR << "[物品配置-称号] id : " << m->first << " 未在Table_Title.txt表中找到" << XEND;
      bCorrect = false;
      continue;
    }

    DWORD type = m->second.getTableInt("GroupID");
    if (type <= ETITLE_TYPE_MIN || type >= ETITLE_TYPE_MAX)
    {
      XERR << "[TitleConfig] id = " << m->first << " type = " << type << " error!" << XEND;
      bCorrect = false;
      continue;
    }

    // attr
    xLuaData& base = m->second.getMutableData("BaseProp");

    auto collectAttr = [&](std::string key, xLuaData &data)
    {
      DWORD dwAttr = RoleDataConfig::getMe().getIDByName(key.c_str());
      if (dwAttr <= EATTRTYPE_MIN || dwAttr >= EATTRTYPE_MAX || EAttrType_IsValid(dwAttr) == false)
      {
        bCorrect = false;
        XERR << "[物品配置-称号] id : " << m->first << " attr : " << key.c_str() << " 未在 Table_RoleData.txt 表中找到" << XEND;
        return;
      }

      UserAttrSvr oAttr;
      oAttr.set_type(static_cast<EAttrType>(dwAttr));
      oAttr.set_value(data.getFloat());
      item->second.vecTitleBase.push_back(oAttr);
    };
    base.foreach(collectAttr);

    item->second.eTitleType = static_cast<ETitleType>(type);
  }

  if (bCorrect)
    XLOG << "[TitleConfig] 成功加载Table_Title.txt" << XEND;
  return bCorrect;
}

bool ItemConfig::loadEquipUpgradeConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_EquipUpgrade.txt"))
  {
    XERR << "[Table_EquipUpgrade.txt], 加载配置失败" << XEND;
    return false;
  }

  m_mapBaseEquip2Equip.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_EquipUpgrade", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    auto equip = m_mapItemCFG.find(m->first);
    if (equip == m_mapItemCFG.end())
    {
      XERR << "[物品配置-装备升级] id :" << m->first << "未在Table_Item.txt中找到" << XEND;
      bCorrect = false;
      continue;
    }

    equip->second.mapUpgradeCFG.clear();

    DWORD dwEvoLv = 1;
    DWORD dwLv = 0;
    DWORD dwCount = 0;
    while (++dwCount < 5)
    {
      stringstream evolv;
      evolv << "ClassDepthLimit_" << dwEvoLv;
      if (m->second.has(evolv.str()) == false)
      {
        ++dwEvoLv;
        continue;
      }
      dwLv = m->second.getTableInt(evolv.str());
      break;
    }

    stringstream smaterial;
    stringstream sbuff;
    DWORD dwIndex = 1;
    while (true)
    {
      smaterial.str("");
      sbuff.str("");
      smaterial << "Material_" << dwIndex;
      sbuff << "BuffID_" << dwIndex;

      if (m->second.has(smaterial.str()) == false)
        break;

      TVecItemInfo vecMaterial;

      auto materialf = [&](const string& key, xLuaData& data)
      {
        ItemInfo oItem;
        oItem.set_id(data.getTableInt("id"));
        oItem.set_count(data.getTableInt("num"));
        combinItemInfo(vecMaterial, TVecItemInfo{oItem});
      };
      m->second.getMutableData(smaterial.str().c_str()).foreach(materialf);
      if (vecMaterial.empty() == true)
        break;

      SEquipUpgradeCFG& rCFG = equip->second.mapUpgradeCFG[dwIndex];

      rCFG.dwLv = dwIndex++;
      rCFG.dwBuffID = m->second.getTableInt(sbuff.str());
      rCFG.dwNpcID = m->second.getTableInt("NpcId");
      if (dwLv != 0 && rCFG.dwLv >= dwLv)
        rCFG.dwEvoReq = dwEvoLv;

      rCFG.oProduct.set_id(m->second.getTableInt("Product"));
      rCFG.oProduct.set_count(1);

      rCFG.vecMaterial.swap(vecMaterial);
      m_mapBaseEquip2Equip[m->first] = rCFG.oProduct.id();
    }
  }

  // 装备升级, 开洞 关联基础装备套装配置
  {
    // add suit cfg by base equip
    for (auto &m : m_mapBaseEquip2Equip)
    {
      auto itbase = m_mapEquipID2SuitID.find(m.first);
      if (itbase == m_mapEquipID2SuitID.end())
        continue;
      auto it = m_mapEquipID2SuitID.find(m.second);
      if (it == m_mapEquipID2SuitID.end())
        m_mapEquipID2SuitID[m.second].insert(itbase->second.begin(), itbase->second.end());
      else
        it->second.insert(itbase->second.begin(), itbase->second.end());
    }

    // add higher equip suit by vid
    map<DWORD, TSetDWORD> mapTempEquip2Suits;
    for (auto m = m_mapEquipID2SuitID.begin(); m != m_mapEquipID2SuitID.end(); ++m)
    {
      const SItemCFG* pCFG = getItemCFG(m->first);
      if (pCFG == nullptr || pCFG->dwVID == 0)
        continue;
      TSetDWORD highids;
      getHigherEquipByVID(pCFG->dwVID, highids);
      if (!highids.empty())
      {
        for (auto &s : highids)
          mapTempEquip2Suits[s].insert(m->second.begin(), m->second.end());
      }
    }
    for (auto &m : mapTempEquip2Suits)
    {
      auto it = m_mapEquipID2SuitID.find(m.first);
      if (it == m_mapEquipID2SuitID.end())
        m_mapEquipID2SuitID[m.first].insert(m.second.begin(), m.second.end());
      else
        it->second.insert(m.second.begin(), m.second.end());
    }
  }

  if (bCorrect)
    XLOG << "[物品配置-装备升级] 成功加载配置Table_EquipUpgrade.txt" << XEND;
  return bCorrect;
}

bool ItemConfig::loadEquipDecomposeConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_EquipDecompose.txt"))
  {
    XERR << "[Table_EquipDecompose.txt], 加载配置失败" << XEND;
    return false;
  }

  for (auto &m : m_mapDecomposeCFG)
    m.second.blInit = false;

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_EquipDecompose", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SEquipDecomposeCFG& rCFG = m_mapDecomposeCFG[m->first];
    rCFG.dwID = m->second.getTableInt("DecomposeID");
    rCFG.dwCost = m->second.getTableInt("Cost");
    rCFG.blInit = true;

    auto materialf = [&](const string& key, xLuaData& data)
    {
      ItemInfo oItem;
      oItem.set_id(data.getTableInt("id"));
      oItem.set_count(data.getTableInt("rate"));
      combinItemInfo(rCFG.vecMaterial, TVecItemInfo{oItem});
    };
    m->second.getMutableData("Material").foreach(materialf);
  }

  if (bCorrect)
    XLOG << "[物品配置-装备溶解] 成功加载配置Table_EquipDecompose.txt" << XEND;
  return bCorrect;
}

bool ItemConfig::loadCardRateConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_CardRate.txt"))
  {
    XERR << "[Table_CardRate.txt], 加载配置失败" << XEND;
    return false;
  }

  for (auto &s : m_vecCardRateCFG)
    s.blInit = false;

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_CardRate", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SCardRateCFG stCFG;
    stCFG.blInit = true;

    auto c = [&](const string& key, xLuaData& data)
    {
      DWORD dwQuality = data.getInt();
      if (dwQuality <= EQUALITYTYPE_MIN || dwQuality >= EQUALITYTYPE_MAX || EQualityType_IsValid(dwQuality) == false)
      {
        XERR << "[物品配置-卡片概率]" << m->first << "quality :" << dwQuality << "不合法" << XEND;
        bCorrect = false;
        return;
      }
      stCFG.vecQuality.push_back(static_cast<EQualityType>(data.getInt()));
    };
    m->second.getMutableData("quality").foreach(c);
    sort(stCFG.vecQuality.begin(), stCFG.vecQuality.end());

    stCFG.dwWriteRate = m->second.getTableInt("WriteCard");
    stCFG.dwGreenRate = m->second.getTableInt("GreenCard");
    stCFG.dwBlueRate = m->second.getTableInt("BlueCard");
    stCFG.dwPurpleRate = m->second.getTableInt("PurpleCard");

    stCFG.dwZeny = m->second.getTableInt("Cost");

    auto rewardf = [&](const string& key, xLuaData& data)
    {
      ItemInfo oItem;
      oItem.set_id(data.getTableInt("1"));
      oItem.set_count(data.getTableInt("2"));
      combinItemInfo(stCFG.vecReward, TVecItemInfo{oItem});
    };
    m->second.getMutableData("Reward").foreach(rewardf);

    m_vecCardRateCFG.push_back(stCFG);
  }

  for (auto &v : m_vecCardRateCFG)
  {
    v.dwGreenRate += v.dwWriteRate;
    v.dwBlueRate += v.dwGreenRate;
    v.dwPurpleRate += v.dwBlueRate;
  }

  if (bCorrect)
    XLOG << "[物品配置-卡片概率] 成功加载配置Table_CardRate.txt" << XEND;
  return bCorrect;
}

bool ItemConfig::loadExchangeConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Exchange.txt"))
  {
    XERR << "[Table_Exchange.txt], 加载配置失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Exchange", table);
  m_mapExchangeItemCFG.clear();
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD dwId = m->first;
    auto n = m_mapExchangeItemCFG.find(dwId);
    if (n != m_mapExchangeItemCFG.end())
    {
      XERR << "[m_mapExchangeItemCFG] id : " << dwId << " duplicated" << XEND;
      bCorrect = false;
      continue;
    }
    SExchangeItemCFG stCFG;
    stCFG.dwId = dwId;
    stCFG.strName = m->second.getTableString("NameZh");
    stCFG.dwMoneyType = m->second.getTableInt("MoneyType");
    stCFG.dwDealNum = m->second.getTableInt("DealNum");
    stCFG.fInRatio = m->second.getTableFloat("InRatio");
    stCFG.fOutRatio = m->second.getTableFloat("OutRatio");
    stCFG.dwBoothfee = m->second.getTableInt("Boothfee");
    stCFG.dwFrozenTime = m->second.getTableInt("FrozenTime");
    stCFG.isOverlap = m->second.getTableInt("Overlap");
    stCFG.dwCategory = m->second.getTableInt("Category");
    stCFG.dwFashionType = m->second.getTableInt("FashionType");
    stCFG.bTrade = m->second.getTableInt("Trade");
    stCFG.dwShowTime = m->second.getTableInt("ShowTime");
    stCFG.dwExchangeNum = m->second.getTableInt("ExchangeNum");
    stCFG.dwRefineCycle = m->second.getTableInt("RefineCycle");
    string strTime = m->second.getTableString("TradeTime");
    if (!strTime.empty())
      parseTime(strTime.c_str(),stCFG.dwTradeTime);
    else
      stCFG.dwTradeTime = 0;
    XDBG <<"[物品配置-交易所] itemid " <<m->first  <<"strtime" << strTime <<"time" <<stCFG.dwTradeTime << XEND;
    if (BaseConfig::getMe().getInnerBranch() == BRANCH_TF)
      stCFG.eAuctionSignupType = static_cast<EAuctionSignupType>(m->second.getTableInt("TFAuction"));
    else
      stCFG.eAuctionSignupType = static_cast<EAuctionSignupType>(m->second.getTableInt("Auction"));

    auto item = m_mapItemCFG.find(m->first);
    if (item != m_mapItemCFG.end())
    {
      for (auto it = item->second.vecEquipPro.begin(); it != item->second.vecEquipPro.end(); it++)
      {
        stCFG.vecJobs.push_back(*it);
      }
    }
    else
    {
      XERR << "[物品配置-交易所] 加载Table_Exchange.txt失败，物品id" << m->first << "未能在Table_Item.txt中找到" << XEND;
      continue;
    }
    
    xLuaData& rData = m->second.getMutableData("Price");
    DWORD priceType = rData.getTableInt("type");
    stCFG.dwPriceType = EPriceType(priceType);
    if (stCFG.dwPriceType == PRICETYPE_SELF)
    {
      stCFG.dwPrice = rData.getTableInt("price");
    }
    else if (stCFG.dwPriceType == PRICETYPE_SUM)
    {
      xLuaData& rPriceSum = rData.getMutableData("pricesum");
      auto priceSumFunc = [&](const string& str, xLuaData& data)
      {
        std::pair<DWORD, DWORD> p;
        p.first = data.getTableInt("itemid");
        p.second = data.getTableInt("count");
        stCFG.vecPriceSum.push_back(p);
      };
      rPriceSum.foreach(priceSumFunc);
    }else {
      //XERR << "[物品配置-交易所] Table_Exchange.txt 错误的 price type 类型"<< stCFG.dwId<<stCFG.dwPriceType << XEND;
      //bCorrect = false;
      //continue;
    }  
    //min price
    rData = m->second.getMutableData("MinPrice");
    {
      DWORD type = rData.getTableInt("type");
      stCFG.minPrice.type = EMinPriceType(type);
      stCFG.minPrice.dwPrice = rData.getTableInt("price");
      stCFG.minPrice.dwComposeId = rData.getTableInt("composeid");
      stCFG.minPrice.dwEquipUpgrade = rData.getTableInt("equip_upgrade_id");
      stCFG.minPrice.ratio = rData.getTableFloat("ratio");
    }
    
    DWORD lv = 0;
    TVecItemInfo vecItemInfo;
    vecItemInfo.reserve(20);
    while (true)
    { 
      lv++;
      const SEquipUpgradeCFG* pUpgradeCfg = item->second.getUpgradeCFG(lv);
      if (pUpgradeCfg == nullptr)
        break;
      stCFG.dwMaxUpgradeLv = lv;
      vecItemInfo.insert(vecItemInfo.end(), pUpgradeCfg->vecMaterial.begin(), pUpgradeCfg->vecMaterial.end());      
      stCFG.mapUpgradePriceSum[lv] = vecItemInfo;
    }    
    m_mapExchangeItemCFG[stCFG.dwId] = stCFG;
  }
  if (bCorrect)
    XLOG << "[物品配置-交易所] 成功加载配置Table_Exchange.txt" << XEND;
  return bCorrect;
}

bool ItemConfig::isFashion(EItemType eType) const
{
  static const vector<EItemType> vecTypes = {
    EITEMTYPE_HEAD, EITEMTYPE_BACK, EITEMTYPE_HAIR, EITEMTYPE_FACE, EITEMTYPE_TAIL,
    EITEMTYPE_WEAPON_LANCE, EITEMTYPE_WEAPON_SWORD, EITEMTYPE_WEAPON_WAND, EITEMTYPE_WEAPON_KNIFE, EITEMTYPE_WEAPON_BOW,
    EITEMTYPE_WEAPON_HAMMER, EITEMTYPE_WEAPON_AXE, EITEMTYPE_WEAPON_BOOK, EITEMTYPE_WEAPON_DAGGER, EITEMTYPE_WEAPON_INSTRUMEMT,
    EITEMTYPE_WEAPON_WHIP, EITEMTYPE_WEAPON_TUBE, EITEMTYPE_WEAPON_FIST, EITEMTYPE_ARMOUR, EITEMTYPE_MOUNT };
  return find(vecTypes.begin(), vecTypes.end(), eType) != vecTypes.end();
}

bool ItemConfig::isEquip(EItemType eType) const
{
  static const vector<EItemType> vecTypes = {
    EITEMTYPE_WEAPON_LANCE, EITEMTYPE_WEAPON_SWORD, EITEMTYPE_WEAPON_WAND, EITEMTYPE_WEAPON_KNIFE, EITEMTYPE_WEAPON_BOW,
    EITEMTYPE_WEAPON_HAMMER, EITEMTYPE_WEAPON_AXE, EITEMTYPE_WEAPON_BOOK, EITEMTYPE_WEAPON_DAGGER, EITEMTYPE_WEAPON_INSTRUMEMT,
    EITEMTYPE_WEAPON_WHIP, EITEMTYPE_WEAPON_TUBE, EITEMTYPE_WEAPON_FIST, EITEMTYPE_ARMOUR, EITEMTYPE_ARMOUR_FASHION, EITEMTYPE_SHIELD, EITEMTYPE_PEARL,
    EITEMTYPE_EIKON, EITEMTYPE_BRACER, EITEMTYPE_BRACELET, EITEMTYPE_TROLLEY, EITEMTYPE_ROBE, EITEMTYPE_SHOES, EITEMTYPE_ACCESSORY
  };
  return find(vecTypes.begin(), vecTypes.end(), eType) != vecTypes.end();
}

bool ItemConfig::isCard(EItemType eType) const
{
  static const vector<EItemType> vecTypes = {
    EITEMTYPE_CARD_WEAPON, EITEMTYPE_CARD_ASSIST, EITEMTYPE_CARD_ARMOUR, EITEMTYPE_CARD_ROBE, EITEMTYPE_CARD_SHOES, EITEMTYPE_CARD_ACCESSORY, EITEMTYPE_CARD_HEAD
  };
  return find(vecTypes.begin(), vecTypes.end(), eType) != vecTypes.end();
}

bool ItemConfig::isUseItem(EItemType eType) const
{
  static const vector<EItemType> vecTypes = {EITEMTYPE_STUFF, EITEMTYPE_STUFFNOCUT, EITEMTYPE_TREASURE, EITEMTYPE_STREASURE,EITEMTYPE_ARROW, EITEMTYPE_GHOSTLAMP,
    EITEMTYPE_USESKILL, EITEMTYPE_MULTITIME,EITEMTYPE_MONTHCARD, EITEMTYPE_QUEST_ONCE, EITEMTYPE_QUEST_TIME, EITEMTYPE_COLLECTION, EITEMTYPE_ACTIVITY, EITEMTYPE_PET_CONSUME, EITEMTYPE_KFC_CODE, EITEMTYPE_FRIEND,
    EITEMTYPE_PET_WEARUNLOCK };
  return find(vecTypes.begin(), vecTypes.end(), eType) != vecTypes.end();
}

bool ItemConfig::isUseNoConsumeItem(EItemType eType) const
{
  static const vector<EItemType> vecTypes = { EITEMTYPE_ARROW, EITEMTYPE_USESKILL, EITEMTYPE_GHOSTLAMP };
  return find(vecTypes.begin(), vecTypes.end(), eType) != vecTypes.end();
}

bool ItemConfig::isPackCheck(EItemType eType) const
{
  if (eType == EITEMTYPE_GOLD || eType == EITEMTYPE_SILVER || eType == EITEMTYPE_DIAMOND || /*eType == EITEMTYPE_GARDEN ||*/ eType == EITEMTYPE_CONTRIBUTE ||
      eType == EITEMTYPE_ASSET || /*eType == EITEMTYPE_FRIENDSHIP ||*/ eType == EITEMTYPE_BASEEXP || eType == EITEMTYPE_JOBEXP || eType == EITEMTYPE_PURIFY ||
      eType == EITEMTYPE_QUESTITEMCOUNT || eType == EITEMTYPE_PORTRAIT || eType == EITEMTYPE_FRAME || eType == EITEMTYPE_HAIR || eType == EITEMTYPE_HAIR_MALE ||
      eType == EITEMTYPE_HAIR_FEMALE || eType == EITEMTYPE_MANUALSPOINT || eType == EITEMTYPE_EYE_MALE || eType == EITEMTYPE_EYE_FEMALE ||
      eType == EITEMTYPE_MANUALPOINT || eType == EITEMTYPE_HONOR || eType == EITEMTYPE_PVPCOIN || eType == EITEMTYPE_LOTTERY || eType == EITEMTYPE_COOKER_EXP ||
      eType == EITEMTYPE_GUILDHONOR || eType == EITEMTYPE_GOLDAPPLE || eType == EITEMTYPE_GETSKILL || eType == EITEMTYPE_POLLY_COIN || eType == EITEMTYPE_PICKEFFECT ||
      eType == EITEMTYPE_PICKEFFECT_1 || eType == EITEMTYPE_DEADCOIN || eType == EITEMTYPE_QUOTA)
    return false;
  return true;
}

bool ItemConfig::isQuest(EItemType eType) const
{
  return eType == EITEMTYPE_QUESTITEM;
}

bool ItemConfig::isGuild(EItemType eType) const
{
  bool b = eType == static_cast<DWORD>(EMONEYTYPE_DIAMOND) || eType == static_cast<DWORD>(EMONEYTYPE_SILVER) || eType == static_cast<DWORD>(EMONEYTYPE_GOLD) ||
      eType == static_cast<DWORD>(EMONEYTYPE_CONTRIBUTE) /*eType == static_cast<DWORD>(EMONEYTYPE_GUILDASSET)*/ || eType == static_cast<DWORD>(EMONEYTYPE_PVPCOIN) ||
      eType == static_cast<DWORD>(EMONEYTYPE_LOTTERY) || eType == static_cast<DWORD>(EMONEYTYPE_GUILDHONOR) ||
      eType == EITEMTYPE_BASEEXP || eType == EITEMTYPE_JOBEXP || eType == EITEMTYPE_PURIFY || eType == EITEMTYPE_QUESTITEMCOUNT || eType == EITEMTYPE_PORTRAIT || eType == EITEMTYPE_FRAME ||
      eType == EITEMTYPE_MANUALSPOINT || eType == EITEMTYPE_MANUALPOINT || eType == EITEMTYPE_HONOR || eType == EITEMTYPE_HAIR || eType == EITEMTYPE_HAIR_MALE ||
      eType == EITEMTYPE_HAIR_FEMALE || eType == EITEMTYPE_COOKER_EXP || eType == EITEMTYPE_EYE_MALE || eType == EITEMTYPE_EYE_FEMALE || eType == EITEMTYPE_EGG ||
      eType == EITEMTYPE_GOLDAPPLE || eType == EITEMTYPE_GETSKILL || eType == EITEMTYPE_PICKEFFECT || eType == EITEMTYPE_PICKEFFECT_1;
  return !b;
}

bool ItemConfig::isPetWearItem(DWORD dwID) const
{
  for (auto &m : m_mapPetWearIDs)
  {
    if (m.second.find(dwID) != m.second.end())
      return true;
  }
  return false;
}

bool ItemConfig::isShowItem(EItemType eType) const
{
  return m_setItemShow.find(eType) != m_setItemShow.end();
}

/*EEquipType ItemConfig::getEquipType(EItemType eItemType) const
{
  if (eItemType >= EITEMTYPE_WEAPON_LANCE && eItemType <= EITEMTYPE_WEAPON_FIST)
    return EEQUIPTYPE_WEAPON;
  if (eItemType == EITEMTYPE_ARMOUR)
    return EEQUIPTYPE_ARMOUR;
  if (eItemType >= EITEMTYPE_SHIELD && eItemType <= EITEMTYPE_TROLLEY)
    return EEQUIPTYPE_SHIELD;
  if (eItemType == EITEMTYPE_ROBE)
    return EEQUIPTYPE_ROBE;
  if (eItemType == EITEMTYPE_SHOES)
    return EEQUIPTYPE_SHOES;
  if (eItemType == EITEMTYPE_ACCESSORY)
    return EEQUIPTYPE_ACCESSORY;
  if (eItemType == EITEMTYPE_HEAD)
    return EEQUIPTYPE_HEAD;
  if (eItemType == EITEMTYPE_BACK)
    return EEQUIPTYPE_BACK;
  if (eItemType == EITEMTYPE_FACE)
    return EEQUIPTYPE_FACE;
  if (eItemType == EITEMTYPE_TAIL)
    return EEQUIPTYPE_TAIL;
  if (eItemType == EITEMTYPE_MOUNT)
    return EEQUIPTYPE_MOUNT;

  return EEQUIPTYPE_MIN;
}*/

EItemType ItemConfig::getItemType(const string& str) const
{
  if (str == "SpearRate")
    return EITEMTYPE_WEAPON_LANCE;
  if (str == "SwordRate")
    return EITEMTYPE_WEAPON_SWORD;
  if (str == "StaffRate")
    return EITEMTYPE_WEAPON_WAND;
  if (str == "KatarRate")
    return EITEMTYPE_WEAPON_KNIFE;
  if (str == "BowRate")
    return EITEMTYPE_WEAPON_BOW;
  if (str == "MaceRate")
    return EITEMTYPE_WEAPON_HAMMER;
  if (str == "AxeRate")
    return EITEMTYPE_WEAPON_AXE;
  if (str == "BookRate")
    return EITEMTYPE_WEAPON_BOOK;
  if (str == "KnifeRate")
    return EITEMTYPE_WEAPON_DAGGER;
  if (str == "InstrumentRate")
    return EITEMTYPE_WEAPON_INSTRUMEMT;
  if (str == "LashRate")
    return EITEMTYPE_WEAPON_WHIP;
  if (str == "PotionRate")
    return EITEMTYPE_WEAPON_TUBE;
  if (str == "GloveRate")
    return EITEMTYPE_WEAPON_FIST;
  if (str == "ArmorRate")
    return EITEMTYPE_ARMOUR;
  if (str == "ShieldRate")
    return EITEMTYPE_SHIELD;
  if (str == "RobeRate")
    return EITEMTYPE_ROBE;
  if (str == "ShoeRate")
    return EITEMTYPE_SHOES;
  if (str == "AccessoryRate")
    return EITEMTYPE_ACCESSORY;
  if (str == "OrbRate")
    return EITEMTYPE_PEARL;
  if (str == "EikonRate")
    return EITEMTYPE_EIKON;
  if (str == "BracerRate")
    return EITEMTYPE_BRACER;
  if (str == "BraceletRate")
    return EITEMTYPE_BRACELET;
  if (str == "TrolleyRate")
    return EITEMTYPE_TROLLEY;
  if (str == "HeadRate")
    return EITEMTYPE_HEAD;
  if (str == "FaceRate")
    return EITEMTYPE_FACE;
  if (str == "MouthRate")
    return EITEMTYPE_MOUTH;
  if (str == "TailRate")
    return EITEMTYPE_TAIL;
  if (str == "WingRate")
    return EITEMTYPE_BACK;

  return EITEMTYPE_MIN;
}

TVecDWORD ItemConfig::getExchangeItemIds(DWORD category, DWORD job, DWORD fashionType)
{
  TVecDWORD vecRet;
  vecRet.reserve(300);
  for (auto it = m_mapExchangeItemCFG.begin(); it != m_mapExchangeItemCFG.end(); ++it)
  {
    if (category != it->second.dwCategory)
      continue;
    if (job && !it->second.isJob(job))
      continue;

    if (fashionType && it->second.dwFashionType != fashionType )
      continue;

    vecRet.push_back(it->first);
  }
  return vecRet;
}

bool ItemConfig::isGoodEnchant(const ItemData& rData)
{
  if (!rData.has_enchant())
    return false;

  //存在属性组合
  if (rData.enchant().extras_size() < 1)
    return false;
  
  const SEnchantCFG* pEnchantCfg = getEnchantCFG(rData.enchant().type());
  if (pEnchantCfg == nullptr)
    return false;
  
  const SItemCFG* pItemCfg = getItemCFG(rData.base().id());
  if (pItemCfg == nullptr)
    return false;
  
  DWORD buffId = rData.enchant().extras(0).buffid();  

  //2016-05023 专注和耐心不算极品附魔
  static TSetDWORD exceptBuffidSet = {500061, 500062, 500063 , 500064 , 500001 ,500002 ,500003 ,500004 };
  auto it = exceptBuffidSet.find(buffId);
  if (it != exceptBuffidSet.end())
    return false;

  bool ret = pEnchantCfg->isGoodEnchant(buffId, pItemCfg->eItemType);

  XLOG << "[交易-判断极品附魔] itemid" << rData.base().id() << "enchanttype" << rData.enchant().type() << "buffid" << buffId << "itemtype" << pItemCfg->eItemType << "ret" << ret << XEND;
  return ret;  
/*
  if (rData.enchant().type() == EENCHANTTYPE_PRIMARY || rData.enchant().type() == EENCHANTTYPE_MEDIUM)
    return false;

  const STradeCFG& rTradeCfg =  MiscConfig::getMe().getTradeCFG();

  for (int i = 0; i < rData.enchant().attrs_size(); ++i)
  {
    const EnchantAttr& rAttr = rData.enchant().attrs(i);

    const SEnchantCFG* pCfg = ItemConfig::getMe().getEnchantCFG(rData.enchant().type());
    if (!pCfg)
      continue;
    const SEnchantAttr* pAttrCfg = pCfg->getEnchantAttr(rAttr.type());
    if (!pAttrCfg)
      continue;

    float rate = ItemConfig::getMe().getEnchantPriceRate(rAttr.type(), rData.base().id());
    if (rate < rTradeCfg.fGoodRate)
      continue;
    
    const RoleData* pData = RoleDataConfig::getMe().getRoleData(rAttr.type());
    if (pData == nullptr)
    {
      XERR << "附魔随机属性 : " << rAttr.type() << " 不合法" << XEND;
      continue;
    }

    float newValue = rAttr.value();
    if (pData->bPercent)
      newValue = newValue / 1000;

    if (newValue >= pAttrCfg->fMax * pAttrCfg->fExpressionOfMaxUp)
      return true;
  }
*/
  return false;
}

bool ItemConfig::isRealAdd(ESource eSource) const
{
  static const vector<ESource> vec = vector<ESource>{
    ESOURCE_PERSON_PUTSTORE, ESOURCE_PERSON_OFFSTORE, ESOURCE_PUBLIC_PUTSTORE, ESOURCE_PUBLIC_OFFSTORE, ESOURCE_PUT_TEMPPACK, ESOURCE_OFF_TEMPPACK,
    ESOURCE_PUT_BARROW, ESOURCE_OFF_BARROW, ESOURCE_TRADE_PUBLICITY_FAILRET};

  auto v = find(vec.begin(), vec.end(), eSource);
  return v == vec.end();
}

bool ItemConfig::isArtifact(EItemType eType) const
{
  if (eType == EITEMTYPE_ARTIFACT_LANCE || eType == EITEMTYPE_ARTIFACT_SWORD || eType == EITEMTYPE_ARTIFACT_WAND || eType == EITEMTYPE_ARTIFACT_KNIFE ||
      eType == EITEMTYPE_ARTIFACT_BOW || eType == EITEMTYPE_ARTIFACT_HAMMER || eType == EITEMTYPE_ARTIFACT_AXE || eType == EITEMTYPE_ARTIFACT_DAGGER ||
      eType == EITEMTYPE_ARTIFACT_FIST || eType == EITEMTYPE_ARTIFACT_INSTRUMEMT || eType == EITEMTYPE_ARTIFACT_WHIP || eType == EITEMTYPE_ARTIFACT_BOOK ||
      eType == EITEMTYPE_ARTIFACT_HEAD || eType == EITEMTYPE_ARTIFACT_BACK)
    return true;
  return false;
}

bool ItemConfig::isSameItemType(EItemType l, EItemType r) const
{
  static const map<EItemType, EItemType> m = map<EItemType, EItemType> {
    make_pair(EITEMTYPE_ARTIFACT_LANCE, EITEMTYPE_WEAPON_LANCE),
    make_pair(EITEMTYPE_ARTIFACT_SWORD, EITEMTYPE_WEAPON_SWORD),
    make_pair(EITEMTYPE_ARTIFACT_WAND, EITEMTYPE_WEAPON_WAND),
    make_pair(EITEMTYPE_ARTIFACT_KNIFE, EITEMTYPE_WEAPON_KNIFE),
    make_pair(EITEMTYPE_ARTIFACT_BOW, EITEMTYPE_WEAPON_BOW),
    make_pair(EITEMTYPE_ARTIFACT_HAMMER, EITEMTYPE_WEAPON_HAMMER),
    make_pair(EITEMTYPE_ARTIFACT_AXE, EITEMTYPE_WEAPON_AXE),
    make_pair(EITEMTYPE_ARTIFACT_DAGGER, EITEMTYPE_WEAPON_DAGGER),
    make_pair(EITEMTYPE_ARTIFACT_FIST, EITEMTYPE_WEAPON_FIST),
    make_pair(EITEMTYPE_ARTIFACT_INSTRUMEMT, EITEMTYPE_WEAPON_INSTRUMEMT),
    make_pair(EITEMTYPE_ARTIFACT_WHIP, EITEMTYPE_WEAPON_WHIP),
    make_pair(EITEMTYPE_ARTIFACT_BOOK, EITEMTYPE_WEAPON_BOOK),
    make_pair(EITEMTYPE_ARTIFACT_HEAD, EITEMTYPE_HEAD),
    make_pair(EITEMTYPE_ARTIFACT_BACK, EITEMTYPE_BACK),
  };
  auto it1 = m.find(l);
  if (it1 != m.end())
    return it1->second == r;
  auto it2 = m.find(r);
  if (it2 != m.end())
    return it2->second == l;
  return false;
}

bool ItemConfig::isArtifactPos(EEquipPos pos) const
{
  static const set<EEquipPos> p = set<EEquipPos>{
    EEQUIPPOS_ARTIFACT, EEQUIPPOS_ARTIFACT_HEAD, EEQUIPPOS_ARTIFACT_BACK,
  };
  return p.find(pos) != p.end();
}

const TSetEquipPos& ItemConfig::getArtifactPos() const
{
  static const set<EEquipPos> p = set<EEquipPos>{
    EEQUIPPOS_ARTIFACT, EEQUIPPOS_ARTIFACT_HEAD, EEQUIPPOS_ARTIFACT_BACK,
  };
  return p;
}

EEquipPos ItemConfig::getArtifactEquipPos(EEquipPos pos) const
{
  static const map<EEquipPos, EEquipPos> m = map<EEquipPos, EEquipPos>{
    make_pair(EEQUIPPOS_ARTIFACT, EEQUIPPOS_WEAPON),
    make_pair(EEQUIPPOS_WEAPON, EEQUIPPOS_ARTIFACT),
    make_pair(EEQUIPPOS_ARTIFACT_HEAD, EEQUIPPOS_HEAD),
    make_pair(EEQUIPPOS_HEAD, EEQUIPPOS_ARTIFACT_HEAD),
    make_pair(EEQUIPPOS_ARTIFACT_BACK, EEQUIPPOS_BACK),
    make_pair(EEQUIPPOS_BACK, EEQUIPPOS_ARTIFACT_BACK),
  };
  auto it = m.find(pos);
  if (it == m.end())
    return EEQUIPPOS_MIN;
  return it->second;
}

DWORD ItemConfig::getLowVidItemId(DWORD vid)
{
  if (vid == 0)
    return 0;
  DWORD t = vid / 1000;
  t = t % 10;
  if (t == 0)
    return 0;
  DWORD newVid = vid - 1000;
  
  for (auto it = m_mapItemCFG.begin(); it != m_mapItemCFG.end(); ++it)
  {
    if (it->second.dwVID == newVid)
    {
      XDBG << "[精炼-低洞] high vid" << vid << "low vid" << newVid << "itemid" << it->first << XEND;
      return it->first;
    }
  }
  return 0;
}

const TSetDWORD& ItemConfig::getDecomposeEquipIDs(DWORD dwDecomposeID) const
{
  const static TSetDWORD e;
  auto m = m_mapDecomposeEquipCFG.find(dwDecomposeID);
  if (m != m_mapDecomposeEquipCFG.end())
    return m->second;
  return e;
}

DWORD ItemConfig::getTitlePreID(DWORD titleid) const
{
  auto it = mapTitle2PreID.find(titleid);
  if(it != mapTitle2PreID.end())
    return it->second;

  return 0;
}

// 检测当前时间是否大于等于配置时间
bool checkDayTime(std::string strDay, DWORD dwYear, DWORD dwMonth, DWORD dwDay)
{
  TVecDWORD vec;
  numTok(strDay, "-", vec);
  if(3 > vec.size())
  {
    XERR << "[ItemConfig] Table_Lottery.txt 配置出错，strDay:" << strDay << XEND;
    return false;
  }

  if(dwYear > vec[0])
    return true;
  if(dwYear < vec[0])
    return false;
  if(dwMonth > vec[1])
    return true;
  if(dwMonth < vec[1])
    return false;
  if(dwDay >= vec[2])
    return true;

  return false;
}

bool ItemConfig::loadLotteryConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Lottery.txt"))
  {
    XERR << "[Table_Lottery.txt], 加载配置失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Lottery", table);
  m_mapLotteryCFG.clear();
  m_setLotteryHeadIDs.clear();
  DWORD key = 0;
  std::string strOnlineTime = "";
  std::string strOfflineTime = "";
  time_t tNow = xTime::getCurSec();
  DWORD dwYear = xTime::getYear(tNow);
  DWORD dwMonth = xTime::getMonth(tNow);
  DWORD dwDay = xTime::getDay(tNow);
  DWORD dwBranch = BaseConfig::getMe().getInnerBranch();
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    strOnlineTime = m->second.getTableString("OnlineTime");
    strOfflineTime = m->second.getTableString("OfflineTime");
    if(!strOnlineTime.empty() && !checkDayTime(strOnlineTime, dwYear, dwMonth, dwDay))
    {
#ifdef _DEBUG
      XLOG << "[ItemConfig] Table_Lottery.txt, 奖品未上架 id:" << m->second.getTableInt("id") << XEND;
#endif
      continue;
    }
    if(!strOfflineTime.empty() && checkDayTime(strOfflineTime, dwYear, dwMonth, dwDay))
    {
#ifdef _DEBUG
      XLOG << "[ItemConfig] Table_Lottery.txt, 奖品已下架 id:" << m->second.getTableInt("id") << XEND;
#endif
      continue;
    }
    DWORD dwItemID = m->second.getTableInt("itemid");
    const SItemCFG* pCFG = getItemCFG(dwItemID);
    if (pCFG == nullptr)
    {
      bCorrect = false;
      XERR << "[Table_Lottery] id :" << m->first << "itemid :" << dwItemID << "未在 Table_Item.txt 表中找到" << XEND;
      continue;
    }
    if (pCFG->isTimeValid() == false)
    {
      XLOG << "[Table_Lottery] id :" << m->first << "当前服务器分支" << dwBranch << "该物品有效期在" << pCFG->dwCardLotteryStartTime << pCFG->dwCardLotteryEndTime << "之内,当前时间为" << tNow << "该物品被过滤" << XEND;
      continue;
    }

    SLotteryItemCFG stItemCFG;
    stItemCFG.dwId = m->second.getTableInt("id");
    stItemCFG.eType = m->second.getTableInt("type");
    stItemCFG.dwItemId = m->second.getTableInt("itemid");
    stItemCFG.dwFemaleItemId = m->second.getTableInt("FemaleItemid");
    stItemCFG.dwCount = m->second.getTableInt("count");
    stItemCFG.dwYear = m->second.getTableInt("Year");
    stItemCFG.dwMonth = m->second.getTableInt("Month");
    stItemCFG.dwWeight = m->second.getTableInt("Weight"); 
    stItemCFG.dwHideWeight = m->second.getTableInt("HideWeight");
    stItemCFG.strRarity = m->second.getTableString("Rarity");
    xLuaData &rPrice = m->second.getMutableData("Price");
    stItemCFG.dwRecoveryItemId = rPrice.getTableInt("1");
    stItemCFG.dwRecoveryCount = rPrice.getTableInt("2");

    stItemCFG.dwBatch = m->second.getTableInt("Batch");
    stItemCFG.dwItemType = m->second.getTableInt("ItemType");
    stItemCFG.dwBasePrice = MiscConfig::getMe().getLotteryCFG().getBasePrice(m->second.getTableInt("BasePrice"));
    stItemCFG.dwCardType = m->second.getTableInt("CardType");

    if(stItemCFG.eType == static_cast<DWORD>(ELotteryType_Magic))
    {
      setFashionID.insert(stItemCFG.dwItemId);
      setFashionID.insert(stItemCFG.dwFemaleItemId);
    }

    if (!stItemCFG.isCurBatch())
      stItemCFG.dwItemType = stItemCFG.convert2Old();

    key = SLotteryItemCFG::getKey(stItemCFG.dwYear, stItemCFG.dwMonth, stItemCFG.dwItemType);
    TMapLotteryCFG& rMapCfg = m_mapLotteryCFG[stItemCFG.eType];
    SLotteryCFG& rLotteryCFG = rMapCfg[key];
    rLotteryCFG.eType = stItemCFG.eType;
    rLotteryCFG.dwMonth = stItemCFG.dwMonth;
    rLotteryCFG.dwYear = stItemCFG.dwYear;

    if (rLotteryCFG.addItemCFG(stItemCFG) == false)
    {
      XERR << "[Table_Lottery.txt], 添加配置失败 key" << key << "id" <<stItemCFG.dwId << "itmeid" << stItemCFG.dwItemId << XEND;
      bCorrect = false;
    }
    else
    {
      rLotteryCFG.dwTotalWeight += stItemCFG.dwWeight;
      rLotteryCFG.dwTotalHideWeight += stItemCFG.dwHideWeight;
    }

    if (stItemCFG.eType == 1)
      m_setLotteryHeadIDs.insert(stItemCFG.dwItemId);
  }

  if (bCorrect)
    XLOG << "[物品配置-扭蛋机] 成功加载配置Table_Lottery.txt" << XEND;
  initLottery();
  return bCorrect;
}

bool ItemConfig::loadLotteryGiveConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_LotteryGive.txt"))
  {
    XERR << "[Table_LotteryGive.txt], 加载配置失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_LotteryGive", table);
  m_mapTime2LotteryGiveCfg.clear();
  m_mapBox2LotteryGiveCfg.clear();
  m_mapLottery2LotteryGiveCfg.clear();

  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SLotteryGiveCFG stGiveCFG;
    stGiveCFG.dwYear = m->second.getTableInt("Year");
    stGiveCFG.dwMonth = m->second.getTableInt("Month");
    stGiveCFG.dwLotteryBoxItemId = m->second.getTableInt("LotterBoxItemId");
    stGiveCFG.dwLotteryItemId = m->second.getTableInt("LotteryItemId");
    m_mapTime2LotteryGiveCfg[stGiveCFG.dwYear*100 + stGiveCFG.dwMonth] = stGiveCFG;
    m_mapBox2LotteryGiveCfg[stGiveCFG.dwLotteryBoxItemId] = stGiveCFG;
    m_mapLottery2LotteryGiveCfg[stGiveCFG.dwLotteryItemId] = stGiveCFG;
  }

  if (bCorrect)
    XLOG << "[物品配置-扭蛋机赠送] 成功加载配置Table_LotteryGive.txt" << XEND;
  return bCorrect;
}


void ItemConfig::getHigherEquipByVID(DWORD vid, TSetDWORD& equipids) const
{
  DWORD basevid = getBaseVID(vid);
  auto it = m_mapBaseVID2Equips.find(basevid);
  if (it == m_mapBaseVID2Equips.end())
    return;
  for (auto &v : it->second)
  {
    if (v.second > vid)
      equipids.insert(v.first);
  }
}

void ItemConfig::getSuitUpEquips(DWORD baseid, TSetDWORD& upEquips) const
{
  auto it = m_mapBaseEquip2Equip.find(baseid);
  if (it != m_mapBaseEquip2Equip.end())
  {
    upEquips.insert(it->second);

    const SItemCFG* pCFG = getItemCFG(it->second);
    if (pCFG != nullptr && pCFG->dwVID)
    {
      TSetDWORD higherids;
      getHigherEquipByVID(pCFG->dwVID, higherids);
      if (!higherids.empty())
        upEquips.insert(higherids.begin(), higherids.end());
    }
  }

  const SItemCFG* pCFG = getItemCFG(baseid);
  if (pCFG != nullptr && pCFG->dwVID)
  {
    TSetDWORD higherids;
    getHigherEquipByVID(pCFG->dwVID, higherids);
    if (!higherids.empty())
      upEquips.insert(higherids.begin(), higherids.end());
  }
}

void ItemConfig::getItemByManualType(TSetDWORD& itemids, EManualType eType) const
{
  for(auto s : m_mapItemCFG)
  {
    if(s.second.swAdventureValue == 0 || ManualConfig::getMe().getManualType(s.second.eItemType) != eType)
      continue;
    itemids.insert(s.first);
  }
}

bool ItemConfig::loadHeadEffectConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_HeadEffect.txt"))
  {
    XERR << "[Table_HeadEffect.txt], 加载配置失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_HeadEffect", table);
  m_mapHeadEffect.clear();
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SHeadEffect stHeadCFG;
    stHeadCFG.dwId = m->first;
    xLuaData& rCondData = m->second.getMutableData("TriggerAction");
    string t = rCondData.getTableString("type");
    if (t == "ActionAnime")
      stHeadCFG.oCond.condType = SHEADCONDTYPE_ACTION;
    else
      stHeadCFG.oCond.condType = SHEADCONDTYPE_MIN;

    auto& tb = rCondData.getMutableData("id").m_table;
    for (auto it = tb.begin(); it != tb.end(); ++it)
    {
      stHeadCFG.oCond.vecParams.push_back(it->second.getInt());
    }
    stHeadCFG.oGMData = m->second.getMutableData("Effect"); 
    m_mapHeadEffect[m->first] = stHeadCFG;
  }

  if (bCorrect)
    XLOG << "[物品配置-头饰音乐] 成功加载配置Table_HeadEffect.txt" << XEND;
  return bCorrect;
}

bool ItemConfig::loadPackSlotLvConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_UnlockSpace.txt"))
  {
    XERR << "[Table_UnlockSpace.txt], 加载配置失败" << XEND;
    return false;
  }

  for (auto &m : m_mapLv2PackSlot)
    m.second.blInit = false;

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_UnlockSpace", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SPackSlotLvCFG& rCFG = m_mapLv2PackSlot[m->first];

    rCFG.dwBaseLv = m->first;
    rCFG.dwMainSlot = m->second.getTableInt("pack");
    rCFG.dwPStoreSlot = m->second.getTableInt("pstore");

    rCFG.blInit = true;
  }

  DWORD dwLastPack = 0;
  DWORD dwLastPStore = 0;
  for (auto &m : m_mapLv2PackSlot)
  {
    m.second.dwMainSlot += dwLastPack;
    dwLastPack = m.second.dwMainSlot;

    m.second.dwPStoreSlot += dwLastPStore;
    dwLastPStore = m.second.dwPStoreSlot;

    XDBG << "[Table_UnlockSpace] lv :" << m.second.dwBaseLv << "mainslot :" << m.second.dwMainSlot << "pstore :" << m.second.dwPStoreSlot << XEND;
  }

  if (bCorrect)
    XLOG << "[Table_UnlockSpace] 成功加载配置Table_UnlockSpace.txt" << XEND;
  return bCorrect;
}

bool ItemConfig::loadPickEffectConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_ItemPickEffect.txt"))
  {
    XERR << "[物品配置-使用道具] 加载配置Table_ItemPickEffect.txt失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_ItemPickEffect", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    auto item = m_mapItemCFG.find(m->first);
    if (item == m_mapItemCFG.end())
    {
      XERR << "[Table_ItemPickEffect] id : " << m->first << " 未在Table_Item.txt表中找到" << XEND;
      bCorrect = false;
      continue;
    }

    item->second.oPickGMData = m->second.getMutableData("Effect");
  }
  if (bCorrect)
    XLOG << "[Table_ItemPickEffect], 成功加载Table_ItemPickEffect.txt" << XEND;
  return bCorrect;
}

bool ItemConfig::loadEquipGenderConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_EquipGender.txt"))
  {
    XERR << "[Table_EquipGender] 加载配置失败" << XEND;
    return false;
  }

  m_mapMaleEquipGender.clear();
  m_mapFemaleEquipGender.clear();

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_EquipGender", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD dwMale = m->second.getTableInt("male");
    DWORD dwFemale = m->second.getTableInt("female");

    const SItemCFG* pMaleCFG = getItemCFG(dwMale);
    if (pMaleCFG == nullptr)
    {
      bCorrect = false;
      XERR << "[Table_EquipGender] male :" << dwMale << "未在 Table_Equip.txt 表中找到" << XEND;
      continue;
    }
    const SItemCFG* pFemaleCFG = getItemCFG(dwFemale);
    if (pFemaleCFG == nullptr)
    {
      bCorrect = false;
      XERR << "[Table_EquipGender] female :" << dwFemale << "未在 Table_Equip.txt 表中找到" << XEND;
      continue;
    }

    m_mapMaleEquipGender[dwMale] = dwFemale;
    m_mapFemaleEquipGender[dwFemale] = dwMale;
  }

  if (bCorrect)
    XLOG << "[Table_EquipGender] 成功加载" << XEND;
  return bCorrect;
}

bool ItemConfig::loadEquipComposeConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_EquipCompose.txt"))
  {
    XERR << "[Table_EquipCompose] 加载配置失败" << XEND;
    return false;
  }

  for (auto &m : m_mapEquipComposeCFG)
    m.second.blInit = false;

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_EquipCompose", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SEquipComposeCFG& rCFG = m_mapEquipComposeCFG[m->first];
    rCFG.dwID = m->first;
    rCFG.blInit = true;
    rCFG.dwZenyCost = m->second.getTableInt("Cost");

    rCFG.vecMaterialEquip.clear();
    auto materialf = [&](const string& key, xLuaData& data)
    {
      ItemData oData;
      oData.mutable_base()->set_id(data.getTableInt("id"));
      oData.mutable_equip()->set_lv(data.getTableInt("lv"));

      if (ItemConfig::getMe().getItemCFG(oData.base().id()) == nullptr)
      {
        bCorrect = false;
        rCFG.blInit = false;
        XERR << "[Table_EquipCompose] id :" << rCFG.dwID << "Material" << oData.ShortDebugString() << "未在 Table_Item.txt 表中找到" << XEND;
        return;
      }
      rCFG.vecMaterialEquip.push_back(oData);
    };
    m->second.getMutableData("Material").foreach(materialf);

    rCFG.vecMaterialItem.clear();
    auto materialitemf = [&](const string& key, xLuaData& data)
    {
      ItemInfo oItem;
      oItem.set_id(data.getTableInt("id"));
      oItem.set_count(data.getTableInt("num"));

      if (oItem.id() == ITEM_ZENY)
      {
        bCorrect = false;
        rCFG.blInit = false;
        XERR << "[Table_EquipCompose] id :" << rCFG.dwID << "MaterialCost中出现zeny配置" << XEND;
        return;
      }
      if (ItemConfig::getMe().getItemCFG(oItem.id()) == nullptr)
      {
        bCorrect = false;
        rCFG.blInit = false;
        XERR << "[Table_EquipCompose] id :" << rCFG.dwID << "MaterialCost" << oItem.ShortDebugString() << "未在 Table_Item.txt 表中找到" << XEND;
        return;
      }
      auto equip = find_if(rCFG.vecMaterialEquip.begin(), rCFG.vecMaterialEquip.end(), [&](const ItemData& rData) -> bool{
        return rData.base().id() == oItem.id();
      });
      if (equip != rCFG.vecMaterialEquip.end())
      {
        bCorrect = false;
        rCFG.blInit = false;
        XERR << "[Table_EquipCompose] id : " << rCFG.dwID << "MaterialCost 中" << oItem.ShortDebugString() << "在Material中出现" << XEND;
        return;
      }
      combinItemInfo(rCFG.vecMaterialItem, oItem);
    };
    m->second.getMutableData("MaterialCost").foreach(materialitemf);
  }

  if (bCorrect)
    XLOG << "[Table_EquipCompose] 加载成功 Table_EquipComose.txt" << XEND;
  return bCorrect;
}

bool ItemConfig::initLottery()
{
  m_mapLotteryItemCFG.clear();
  m_mapItem2BasePrice.clear();
  m_mapType2LotteryPool.clear();
  m_mapCardType2Rate.clear();

  DWORD dwCardWeightTotal = 0;
  std::map<DWORD, QWORD> mapCardType2Weight;  // <type, weight>
  // lottery
  for (auto& m : m_mapLotteryCFG)
  {
    for (auto& m2 : m.second)
    {
      DWORD weightOffset = 0;
      DWORD hideWeightOffset = 0;
      for (auto& subM : m2.second.mapItemCFG)
      {
        const SItemCFG* pItemCFG = getItemCFG(subM.second.dwItemId);
        if (pItemCFG == nullptr)
        {
          XERR << "[物品配置-扭蛋机] itemid :" << subM.second.dwItemId << "未在 Table_Item.txt 表中找到" << XEND;
          continue;
        }
        weightOffset += subM.second.dwWeight;
        subM.second.dwWeightOffSet = weightOffset;
        subM.second.dwRate = subM.second.dwWeight / (float)m2.second.dwTotalWeight * LOTTERY_PER_MAX;
#ifdef _DEBUG
        XLOG << "[扭蛋-配置] Lotter.txt type" << m.first << "key" << m2.first << "itemid" << subM.second.dwItemId << "weight" << subM.second.dwWeight << "totalweight" << m2.second.dwTotalWeight << "rate" << subM.second.dwRate <<"itemtype"<< subM.second.dwItemType << XEND;
#endif

        hideWeightOffset += subM.second.dwHideWeight;
        subM.second.dwHideWeightOffSet = hideWeightOffset;

        DWORD key = subM.second.dwItemId * 100 + subM.second.eType;
        m_mapLotteryItemCFG[key] = subM.second;
        if (subM.second.dwFemaleItemId)
        {
          DWORD key = subM.second.dwFemaleItemId * 100 + subM.second.eType;
          m_mapLotteryItemCFG[key] = subM.second;
        }

        if(subM.second.dwBasePrice)
        {
          DWORD key = LotteryPool::getKey(m.first, subM.second.dwYear, subM.second.dwMonth);
          LotteryPool& pool = m_mapType2LotteryPool[key];
          pool.add(subM.second);
          m_mapItem2BasePrice[subM.second.dwItemId] = subM.second.dwBasePrice;
          if(subM.second.dwFemaleItemId)
            m_mapItem2BasePrice[subM.second.dwFemaleItemId] = subM.second.dwBasePrice;
        }

        if(ELotteryType_Card == m.first)
        {
          dwCardWeightTotal += subM.second.dwWeight;
          mapCardType2Weight[subM.second.dwCardType] += subM.second.dwWeight;
        }
      }
    }
  }

  for(auto& m : mapCardType2Weight)
  {
    m_mapCardType2Rate[m.first] = m.second*LOTTERY_PER_MAX / dwCardWeightTotal;
    XDBG << "[扭蛋-配置] cardType:" << m.first << "rate:" << m.second << XEND;
  }

  return true;
}

bool ItemConfig::getItemFromLotteryPool(DWORD& dwOutItem, DWORD& dwOutCount, DWORD& dwOutRate, EGender gender, DWORD key, DWORD dwCost, TVecDWORD& vecItemDels)
{
  auto it = m_mapType2LotteryPool.find(key);
  if(m_mapType2LotteryPool.end() == it)
    return false;

  return it->second.get(dwOutItem, dwOutCount, dwOutRate, gender, dwCost, vecItemDels);
}

const SLotteryGiveCFG* ItemConfig::getLotteryBoxYearMonth(TVecItemData& rVec) const
{
  for (auto&v : rVec)
  {
    const SLotteryGiveCFG*pCfg = getLotteryGiveCFGByBoxId(v.base().id());
    if (pCfg)
      return pCfg;
  }
  return nullptr;
}

DWORD ItemConfig::getGenderEquip(EGender eGender, DWORD dwEquip) const
{
  if (eGender <= EGENDER_MIN || eGender >= EGENDER_MAX || EGender_IsValid(eGender) == false)
    return 0;

  if (eGender == EGENDER_MALE)
    eGender = EGENDER_FEMALE;
  else if (eGender == EGENDER_FEMALE)
    eGender = EGENDER_MALE;

  if (eGender == EGENDER_MALE)
  {
    auto m = m_mapMaleEquipGender.find(dwEquip);
    return m != m_mapMaleEquipGender.end() ? m->second : 0;
  }
  else if (eGender == EGENDER_FEMALE)
  {
    auto m = m_mapFemaleEquipGender.find(dwEquip);
    return m != m_mapFemaleEquipGender.end() ? m->second : 0;
  }

  return 0;
}

const SEquipComposeCFG* ItemConfig::getEquipComposeCFG(DWORD dwID) const
{
  auto m = m_mapEquipComposeCFG.find(dwID);
  if (m != m_mapEquipComposeCFG.end() && m->second.blInit)
    return &m->second;
  return nullptr;
}

void ItemConfig::fillLotteryCardRate(Cmd::LotteryRateQueryCmd& cmd) const
{
  for(auto& m : m_mapCardType2Rate)
  {
    Cmd::LotteryRateInfo* pInfo = cmd.add_infos();
    if(!pInfo)
    {
      XERR << "[扭蛋-卡片概率] add infos failed! type:" << m.first << "rate:" << m.second << XEND;
      return;
    }

    pInfo->set_type(m.first);
    pInfo->set_rate(m.second);
  }
}

void ItemConfig::timer(DWORD curSec)
{
  // card load
  if (curSec > m_dwCardCFGLoadTick)
  {
    loadCardConfig();
    loadLotteryConfig();
  }
}

