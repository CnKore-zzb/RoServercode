#include "Share.h"
#include "SceneUser.h"
#include "UserEvent.pb.h"

void SShareCalc::fromData(const Cmd::ShareCalc& protoData)
{
  type = protoData.type();
  
  for (int i = 0; i < protoData.items_size(); ++i)
  {
    const ShareCalcItem& rD = protoData.items(i);
    QWORD key = 0;
    convert32To64(rD.high_key(), rD.low_key(), key);
    items[key] = rD;
    QWORD value = 0;
    convert32To64(rD.high_value(), rD.low_value(), value);
    XDBG << "[分享-加载] calcdata" << type << "key" << key << "value" << value << XEND;
  }
  //if (protoData.items_size() > 0)
  DWORD dwCount = protoData.items_size() > 4 ? 4 : protoData.items_size();
  for (DWORD d = 0; d < dwCount; ++d)
    vecMax.push_back(protoData.items(d));
}

void SShareCalc::toData(Cmd::ShareCalc* pProtoData)
{
  if (pProtoData == nullptr)
    return;
    
  pProtoData->set_type(type);

  //sort  只保存最前面的50条
  std::vector<ShareCalcItem> vecCalc;
  vecCalc.reserve(items.size());
  for (auto &v : items)
  {
    vecCalc.push_back(v.second);
  }  
  
  if (vecCalc.size() > MAX_COUNT)
  {//只排序前 MAX_COUNT 个
    std::partial_sort(vecCalc.begin(), vecCalc.begin() + MAX_COUNT, vecCalc.end(), CmpByValue());
  }
  else
  {
    std::sort(vecCalc.begin(), vecCalc.end(), CmpByValue());
  }
  
  int i = 0;
  for (auto it = vecCalc.begin(); it != vecCalc.end(); ++it)
  {    
    if (i > MAX_COUNT)
      break;    
    ShareCalcItem* pD = pProtoData->add_items();
    if (pD)
      pD->CopyFrom(*it);
    i++;
    if (pD)
    {
      XDBG << "[分享-calcdata] type" << type << "key" << pD->high_key() <<pD->low_key()<< "value"<<pD->high_value() << pD->low_value() << XEND;
    }
  } 
  DWORD dwCount = pProtoData->items_size() > 4 ? 4 : pProtoData->items_size();
  for (DWORD d = 0; d < dwCount; ++d)
    vecMax.push_back(pProtoData->items(d));
}

void SShareCalc::add(QWORD key, QWORD value)
{ 
  auto it = items.find(key);
  if (it != items.end())
  {
    QWORD newValue;
    convert32To64(it->second.high_value(), it->second.low_value(), newValue);
    newValue = newValue + value;

    DWORD h32, l32;
    convert64To32(newValue, h32, l32);
    if (h32)
      it->second.set_high_value(h32);
    if (l32)
      it->second.set_low_value(l32);

    XDBG << "[分享-新加] type" << type << "key" << key << "value" << value <<"aftervalue"<<newValue << XEND;
    return;
  }
  ShareCalcItem data;
  DWORD h32, l32;
  convert64To32(key, h32, l32);
  if (h32)
    data.set_high_key(h32);
  if (l32)
    data.set_low_key(l32);
  
  convert64To32(value, h32, l32);
  if (h32)
    data.set_high_value(h32);
  if (l32)
    data.set_low_value(l32);  
  items[key] = data;
  XDBG << "[分享-新加] type" << type << "key" << key << "value" << value << XEND;
}

Share::Share(SceneUser* pUser) : m_pUser(pUser)
{
}

Share::~Share()
{
}

bool Share::load(const BlobShare& data)
{
  for (int i = 0; i < data.normaldata_size(); ++i)
  {
    const ShareNormal& rNormalData = data.normaldata(i);    
    m_mapNomalData[rNormalData.type()] = rNormalData.value();
  }
  
  for (int i = 0; i < data.calcdata_size(); ++i)
  {
    const ShareCalc& rData = data.calcdata(i);
    SShareCalc &rS = m_mapCalcData[rData.type()];
    rS.fromData(rData);
  }

  m_oFirstMvp = data.firstmvp();
  m_strFirstPhoto = data.firstphoto();
  m_oFirstHand.CopyFrom(data.firsthand());
  m_oFirstCarrier.CopyFrom(data.firstcarrier());
  m_oMaxTradeBuy.CopyFrom(data.tradebuy());
  m_oMaxTradeSell.CopyFrom(data.tradesell());
  m_oMaxDamage.CopyFrom(data.maxdamage());  

  
  for (int i = 0; i < data.mystery_box_size(); ++i)
  {
    m_setMysteryBox.insert(data.mystery_box(i));
  }

  XLOG << "[分享-加载] id" << m_pUser->id << "加载了 普通" << m_mapNomalData.size() << "条" << "calcdata" << m_mapCalcData.size() << "条" << XEND;
  return true;
}

bool Share::save(BlobShare* data)
{
  if (data == nullptr)
    return false;

  for (auto it = m_mapNomalData.begin(); it != m_mapNomalData.end(); ++it)
  {
    ShareNormal*pD = data->add_normaldata();
    if (pD)
    {
      pD->set_type(it->first);
      pD->set_value(it->second);
    }
  }

  //friend num
  {
    ShareNormal*pD = data->add_normaldata();
    if (pD)
    {
      pD->set_type(ESHAREDATATYPE_N_FRIENDCOUNT);
      pD->set_value(m_pUser->getSocial().getRelationCount(ESOCIALRELATION_FRIEND));
    }
  }

  for (auto it = m_mapCalcData.begin(); it != m_mapCalcData.end(); ++it)
  {
    ShareCalc* pC = data->add_calcdata();
    if (pC)
    {
      it->second.toData(pC);
    }
  }
  
  data->set_firstphoto(m_strFirstPhoto);

  if (data->mutable_firstmvp())
    data->mutable_firstmvp()->CopyFrom(m_oFirstMvp);
  if (data->mutable_firsthand())
    data->mutable_firsthand()->CopyFrom(m_oFirstHand);
  if (data->mutable_firstcarrier())
    data->mutable_firstcarrier()->CopyFrom(m_oFirstCarrier);
  if (data->mutable_tradebuy())
    data->mutable_tradebuy()->CopyFrom(m_oMaxTradeBuy);
  if (data->mutable_tradesell())
    data->mutable_tradesell()->CopyFrom(m_oMaxTradeSell);
  if (data->mutable_maxdamage())
    data->mutable_maxdamage()->CopyFrom(m_oMaxDamage);
  
  int i = 0;
  for (auto &v : m_setMysteryBox)
  {
    if (i++ > 100)
      break;
    data->add_mystery_box(v);
  }

  XLOG << "[分享-保存] id" << m_pUser->id << "加载了 普通" << m_mapNomalData.size() << "条" << "calcdata" << m_mapCalcData.size() << "条" << XEND;
  return true;
}

void Share::addNormalData(EShareDataType eType, QWORD value)
{
  if (eType <= ESHAREDATATYPE_MIN || eType >= ESHAREDATATYPE_MAX)
    return;

  //总和 sum
  static std::set<EShareDataType> sumSetType = { 
    ESHAREDATATYPE_S_BATTLETIME,
    ESHAREDATATYPE_S_KILLMONSTER,
    ESHAREDATATYPE_S_MVPCOUNT,
    ESHAREDATATYPE_S_MINICOUNT,
    ESHAREDATATYPE_S_MOVEDIS,
    ESHAREDATATYPE_S_PHOTOCOUNT,
    ESHAREDATATYPE_S_TRADECOST,
    ESHAREDATATYPE_S_TRADEGAIN,
    ESHAREDATATYPE_S_REFINECOUNT,
    ESHAREDATATYPE_S_REFINESUCCESS,
    ESHAREDATATYPE_S_REFINEDAMAGE,
    ESHAREDATATYPE_S_ENCHANTCOUNT,
    ESHAREDATATYPE_S_ENCHANTCOST,
    ESHAREDATATYPE_S_HUEDIE,
    ESHAREDATATYPE_S_CANGYING,
    ESHAREDATATYPE_S_LOGINCOUNT,
    ESHAREDATATYPE_S_BE_PRO_1_TIME,
    ESHAREDATATYPE_S_BE_PRO_2_TIME,
    ESHAREDATATYPE_S_BE_PRO_3_TIME,
  };

  if (sumSetType.find(eType) != sumSetType.end())
  {
    m_mapNomalData[eType] += value;
  }

  //当前 new
  static std::set<EShareDataType> newSetType = {
    ESHAREDATATYPE_N_FRIENDCOUNT
  };

  if (newSetType.find(eType) != newSetType.end())
  {
    m_mapNomalData[eType] = value;
  }

  //最大 MAX
  static std::set<EShareDataType> maxSetType = {
    ESHAREDATATYPE_MAX_TOWER
  };

  if (maxSetType.find(eType) != maxSetType.end())
  {
    QWORD oldValue = m_mapNomalData[eType];
    if (oldValue < value)
      m_mapNomalData[eType] = value;
  }
  
  XDBG << "[分享-新加] id" << m_pUser->id << "type" << eType << "value" << value << XEND;
}

void Share::addCalcData(EShareDataType eType, QWORD key, QWORD value)
{
  if (eType <= ESHAREDATATYPE_MIN || eType >= ESHAREDATATYPE_MAX)
    return;
  
  auto it = m_mapCalcData.find(eType);
  if (it == m_mapCalcData.end())
  {
    SShareCalc calc;
    calc.type = eType;
    it = m_mapCalcData.insert(std::make_pair(eType, calc)).first;
  }
  
  if (it != m_mapCalcData.end())
  {
    it->second.add(key, value);
  }
  XDBG << "[分享-新加] id" << m_pUser->id << "type" << eType <<"key" << key << "value" << value << XEND;
}

void Share::onKillMvp(DWORD npcId, DWORD mapId)
{
  if (m_oFirstMvp.has_mvpid())
    return;
  m_oFirstMvp.set_mvpid(npcId);
  XLOG << "[分享-击杀mvp-新加] id" << m_pUser->id << "npcid"<<npcId<< XEND;
}

void Share::onHand(QWORD otherId, DWORD mapId)
{
  if (otherId == 0)
    return;
  if (m_oFirstHand.has_otherid())
    return;
  m_oFirstHand.set_otherid(otherId);
  XLOG << "[分享-第一次牵手-新加] id" << m_pUser->id << "otherid" << otherId << XEND;
}

void Share::onCarrier(QWORD otherId, DWORD mapId)
{
  if (otherId == 0)
    return;
  if (m_oFirstCarrier.has_otherid())
    return;
  m_oFirstCarrier.set_otherid(otherId);
}

void Share::onTradeBuy(DWORD itemId, QWORD totalPrice, DWORD refineLv)
{
  if (m_oMaxTradeBuy.has_itemid())
  {
    if (totalPrice <= m_oMaxTradeBuy.total_price())
    {
      return;
    }
  } 

  m_oMaxTradeBuy.set_itemid(itemId);
  m_oMaxTradeBuy.set_total_price(totalPrice);
  m_oMaxTradeBuy.set_refine_lv(refineLv);
  XDBG << "[分享-交易所最大购买-新加] " << m_pUser->id << "itemid" << itemId << "totalprice" << totalPrice << "refinelv" << refineLv << XEND;
}

void Share::onTradeSell(DWORD itemId, QWORD totalPrice, DWORD refineLv)
{
  if (m_oMaxTradeSell.has_itemid())
  {
    if (totalPrice <= m_oMaxTradeSell.total_price())
    {
      return;
    }
  }
  m_oMaxTradeSell.set_itemid(itemId);
  m_oMaxTradeSell.set_total_price(totalPrice);
  m_oMaxTradeSell.set_refine_lv(refineLv);
  XDBG << "[分享-交易所最大出售-新加] " << m_pUser->id << "itemid" <<itemId << "totalprice" << totalPrice << "refinelv" << refineLv << XEND;
}

void Share::onHurt(const std::string& target, DWORD value)
{
  if (m_oMaxDamage.has_damage() && m_oMaxDamage.damage() > value)
  {
    return;
  }
  
  m_oMaxDamage.set_target(target);
  m_oMaxDamage.set_damage(value);
  XDBG << "[分享-最大伤害-新加] " << m_pUser->id << "target" << target << "value" << target <<XEND;
}

void Share::onOpenMysteryBox(DWORD itemId)
{
  m_setMysteryBox.insert(itemId);
}

DWORD Share::getNormalData(EShareDataType eType) const
{
  auto m = m_mapNomalData.find(eType);
  if (m != m_mapNomalData.end())
    return m->second;
  return 0;
}

QWORD Share::getMaxCount(EShareDataType eType)
{
  auto m = m_mapCalcData.find(eType);
  if (m == m_mapCalcData.end())
    return 0;

  QWORD qwValue = 0;
  for (auto &item : m->second.items)
  {
    QWORD newValue = 0;
    m->second.convert32To64(item.second.high_value(), item.second.low_value(), newValue);
    if (newValue > qwValue)
      qwValue = newValue;
  }
  return qwValue;
}

QWORD Share::getMaxKey(EShareDataType eType)
{
  auto m = m_mapCalcData.find(eType);
  if (m != m_mapCalcData.end())
    return m->second.items.size();
  return 0;
}

bool Share::collectMostCharID(EShareDataType eType, DWORD dwNum, TSetQWORD& setCharIDs)
{
  setCharIDs.clear();
  auto m = m_mapCalcData.find(eType);
  if (m == m_mapCalcData.end())
    return false;

  DWORD dwCount = m->second.vecMax.size() > dwNum ? dwNum : m->second.vecMax.size();
  for (DWORD d = 0; d < dwCount; ++d)
  {
    QWORD key = 0;
    m->second.convert32To64(m->second.vecMax[d].high_key(), m->second.vecMax[d].low_key(), key);
    setCharIDs.insert(key);
  }

  return true;
}

bool Share::collectShareCharID(EShareDataType eType, TSetQWORD& setCharIDs)
{
  setCharIDs.clear();
  auto m = m_mapCalcData.find(eType);
  if (m == m_mapCalcData.end())
    return false;

  for (auto &calc : m->second.items)
  {
    QWORD key = 0;
    m->second.convert32To64(calc.second.high_key(), calc.second.low_key(), key);
    setCharIDs.insert(key);
  }

  return true;
}

