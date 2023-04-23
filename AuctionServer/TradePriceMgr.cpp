#include "TradePriceMgr.h"
#include "RedisManager.h"
#include "xServer.h"
#include "CommonConfig.h"
#include "ItemConfig.h"

extern xServer* thisServer;

// GChar
TradePriceMgr::TradePriceMgr()
{
  m_dwRegionID = thisServer->getRegionID();
  m_strRedisKey.clear();
  m_strRedisKey = RedisManager::getMe().getTradePriceKey(m_dwRegionID, EREDISKEYTYPE_TRADE_PRICE);
}

TradePriceMgr::~TradePriceMgr()
{
}

bool TradePriceMgr::get()
{
  if (!m_dwRegionID)
  {
    XDBG << "[TradePriceMgr-加载]" << "失败" << m_dwRegionID << XEND;
    return false;
  }

  m_oData.clear();

  auto& exchangeItemMap = ItemConfig::getMe().getExchangeItemCFGMap();
  for (auto it = exchangeItemMap.begin(); it != exchangeItemMap.end(); ++it)
  {
    const SExchangeItemCFG& rCfg = it->second;
    if (EAUCTIONSIGNUPTYPE_NONE != rCfg.eAuctionSignupType)
    {
      char str[64] = { 0 };
      snprintf(str, sizeof(str), "%u", rCfg.dwId);
      m_oData.setData(str, "");
    }
  }


  if (RedisManager::getMe().getHashAll(m_strRedisKey, m_oData) == false)
  {
    XERR << "[TradePriceMgr-加载]" << m_strRedisKey << "读取redis数据库失败" << XEND;
    return false;
  }
  
  XLOG << "[TradePriceMgr-加载]" << m_strRedisKey << "成功从redis加载" << XEND;
  return true;
}

bool TradePriceMgr::getPrice(std::map<DWORD/*itemid*/,QWORD/*price*/>& mapItem)
{
  if (mapItem.empty())
    return false;

  if (!get())
    return false;
  
  for (auto&m : mapItem)
  {
    char str[64] = { 0 };
    snprintf(str, sizeof(str), "%u", m.first);

    m.second = m_oData.getTableQWORD(str);
    if (m.second == 0)
    {
      XERR << "[拍卖行-交易所价格] 获取不到交易所价格，物品id" << m.first << XEND;
      continue;
    }
  }
  return true;
}

QWORD TradePriceMgr::getPrice(DWORD itemId)
{
  if(!get())
    return 0;

  char str[64];
  bzero(str, sizeof(str));
  snprintf(str, sizeof(str), "%u", itemId);
  return m_oData.getTableQWORD(str);
}
