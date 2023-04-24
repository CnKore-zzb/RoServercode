#include "ExchangeShopCondition.h"
#include "SceneUser.h"
#include "ExchangeShop.h"

BaseExchangeShopCondition::BaseExchangeShopCondition()
{
}

BaseExchangeShopCondition::~BaseExchangeShopCondition()
{
}

BaseLevelExchangeCondition::BaseLevelExchangeCondition()
{
  condType = EEXCHANGECONDITION_BASE_LEVEL;
}

BaseLevelExchangeCondition::~BaseLevelExchangeCondition()
{
}

bool BaseLevelExchangeCondition::init(xLuaData &data)
{
  m_dwMinLevel = data.getTableInt("a");
  m_dwMaxLevel = data.getTableInt("b");
  if (m_dwMaxLevel == 0)
  {
    m_dwMaxLevel = DWORD_MAX;
  }
  if (m_dwMinLevel >= m_dwMaxLevel)
    return false;
  return true;
}

bool BaseLevelExchangeCondition::checkOk(SceneUser* pUser)
{
  if (pUser == nullptr)
    return false;
  if (pUser->getLevel() >= m_dwMinLevel && pUser->getLevel() < m_dwMaxLevel)
    return true;
  return false;
}

JobLevelExchangeCondition::JobLevelExchangeCondition()
{
  condType = EEXCHANGECONDITION_JOB_LEVEL;
}

JobLevelExchangeCondition::~JobLevelExchangeCondition()
{
}

bool JobLevelExchangeCondition::init(xLuaData &data)
{
  m_dwMinLevel = data.getTableInt("a");
  m_dwMaxLevel = data.getTableInt("b");
  if (m_dwMaxLevel == 0)
  { 
    m_dwMaxLevel = DWORD_MAX;
  }   
  if (m_dwMinLevel >= m_dwMaxLevel)
    return false;
  return true;
}

bool JobLevelExchangeCondition::checkOk(SceneUser* pUser)
{
  if (pUser == nullptr)
    return false;
  //if (pUser->getMaxCurJobLv() >= m_dwMinLevel && pUser->getMaxCurJobLv() < m_dwMaxLevel)
  //  return true;
  SceneFighter* pFighter = pUser->getFighter();
  if (pFighter == nullptr)
    return false;
  DWORD dwJobLv = pFighter->getJobLv();
  return dwJobLv >= m_dwMinLevel && dwJobLv < m_dwMaxLevel;
}

HasExchangedExchangeCondition::HasExchangedExchangeCondition()
{
  condType = EEXCHANGECONDITION_HAS_EXCHANGED;
}

HasExchangedExchangeCondition::~HasExchangedExchangeCondition()
{
}

bool HasExchangedExchangeCondition::init(xLuaData &data)
{
  m_dwShopItem = data.getTableInt("a");
  if (m_dwShopItem == 0)
    return false;
  return true;
}

bool HasExchangedExchangeCondition::checkOk(SceneUser* pUser)
{
  if (pUser == nullptr)
    return false;
  if (m_dwShopItem == 0)
    return false;
  return pUser->getExchangeShop().checkHasExchangedItem(m_dwShopItem);
}

HigherShopItemExchangeCondition::HigherShopItemExchangeCondition()
{
  condType = EEXCHANGECONDITION_HIGHER_SHOPITEM; 
}

HigherShopItemExchangeCondition::~HigherShopItemExchangeCondition()
{
}

bool HigherShopItemExchangeCondition::init(xLuaData &data)
{
  m_dwShopItem = data.getTableInt("a");
  if (m_dwShopItem == 0)
    return false;
  return true;
}

bool HigherShopItemExchangeCondition::checkOk(SceneUser* pUser)
{
  return false;
}

MedalCountExchangeCondition::MedalCountExchangeCondition()
{
  condType = EEXCHANGECONDITION_MEDAL_COUNT;
}

MedalCountExchangeCondition::~MedalCountExchangeCondition()
{
}

bool MedalCountExchangeCondition::init(xLuaData &data)
{
  m_dwMinCount = data.getTableInt("a");
  m_dwMaxCount = data.getTableInt("b");
  if (m_dwMaxCount == 0)
  {
    m_dwMaxCount = DWORD_MAX;
  }
  if (m_dwMinCount >= m_dwMaxCount)
    return false;
  return true;
}

bool MedalCountExchangeCondition::checkOk(SceneUser* pUser)
{
  if (pUser == nullptr)
    return false;

  DWORD itemCount = pUser->getExchangeShop().getItemGetCount(ITEM_MEDAL);
  if (itemCount >= m_dwMinCount && itemCount < m_dwMaxCount)
    return true;
  return false;
}

BattleTimeExchangeCondition::BattleTimeExchangeCondition()
{
  condType = EEXCHANGECONDITION_BATTLE_TIME;
}

BattleTimeExchangeCondition::~BattleTimeExchangeCondition()
{
}

bool BattleTimeExchangeCondition::init(xLuaData &data)
{
  m_dwTime = data.getTableInt("a") * 60;
  return true;
}

bool BattleTimeExchangeCondition::checkOk(SceneUser* pUser)
{
  if (pUser == nullptr)
    return false;

  return m_dwTime <= pUser->getBattleTime();
}

bool ForeverExchangeCondition::init(xLuaData& data)
{
  m_bSuccess = data.getTableInt("a") == 1;
  return true;
}

