#pragma once

#include "xDefine.h"
#include "xLuaTable.h"

enum EExchangeShopConditionType
{
  EEXCHANGECONDITION_NONE             = 0,
  EEXCHANGECONDITION_BASE_LEVEL       = 1,  //base等级
  EEXCHANGECONDITION_JOB_LEVEL        = 2,   //job等级
  EEXCHANGECONDITION_HAS_EXCHANGED    = 3, //是否购买过
  EEXCHANGECONDITION_HIGHER_SHOPITEM  = 4, //是否有同类型中更高ID的商品
  EEXCHANGECONDITION_MEDAL_COUNT      = 5, //金质勋章数量
  EEXCHANGECONDITION_BATTLE_TIME      = 6, //战斗时长
  EEXCHANGECONDITION_FOREVER          = 7,  // 永久
};

class SceneUser;
class BaseExchangeShopCondition
{
  public:
    BaseExchangeShopCondition();
    virtual ~BaseExchangeShopCondition();

    EExchangeShopConditionType getCondType() { return condType; }

  public:
    virtual bool init(xLuaData &data) = 0;
    virtual bool checkOk(SceneUser*) = 0;
  protected:
    EExchangeShopConditionType condType = EEXCHANGECONDITION_NONE;
};

class BaseLevelExchangeCondition : public BaseExchangeShopCondition
{
  public:
    BaseLevelExchangeCondition();
    virtual ~BaseLevelExchangeCondition();

  public:
    virtual bool init(xLuaData &data);
    virtual bool checkOk(SceneUser* pUser);

  private:
    DWORD m_dwMinLevel;
    DWORD m_dwMaxLevel;
};

class JobLevelExchangeCondition : public BaseExchangeShopCondition
{
  public:
    JobLevelExchangeCondition();
    virtual ~JobLevelExchangeCondition();

  public:
    virtual bool init(xLuaData &data);
    virtual bool checkOk(SceneUser* pUser);

  private:
    DWORD m_dwMinLevel;
    DWORD m_dwMaxLevel;
};

class HasExchangedExchangeCondition : public BaseExchangeShopCondition
{
  public:
    HasExchangedExchangeCondition();
    virtual ~HasExchangedExchangeCondition();

  public:
    virtual bool init(xLuaData &data);
    virtual bool checkOk(SceneUser* pUser);
  private:
    DWORD m_dwShopItem;
};

class HigherShopItemExchangeCondition : public BaseExchangeShopCondition
{
  public:
    HigherShopItemExchangeCondition();
    virtual ~HigherShopItemExchangeCondition();

  public:
    virtual bool init(xLuaData &data);
    virtual bool checkOk(SceneUser* pUser);

  private:
    DWORD m_dwShopItem;
};

class MedalCountExchangeCondition : public BaseExchangeShopCondition
{
  public:
    MedalCountExchangeCondition();
    virtual ~MedalCountExchangeCondition();

  public:
    virtual bool init(xLuaData &data);
    virtual bool checkOk(SceneUser* pUser);

  private:
    DWORD m_dwMinCount;
    DWORD m_dwMaxCount;
};

class BattleTimeExchangeCondition : public BaseExchangeShopCondition
{
  public:
    BattleTimeExchangeCondition();
    virtual ~BattleTimeExchangeCondition();

  public:
    virtual bool init(xLuaData &data);
    virtual bool checkOk(SceneUser* pUser);

  private:
    DWORD m_dwTime;
};

class ForeverExchangeCondition : public BaseExchangeShopCondition
{
  public:
    ForeverExchangeCondition() { condType = EEXCHANGECONDITION_FOREVER; }
    virtual ~ForeverExchangeCondition() {}

  public:
    virtual bool init(xLuaData& data);
    virtual bool checkOk(SceneUser* pUser) { return m_bSuccess; }
  private:
    bool m_bSuccess = false;
};

