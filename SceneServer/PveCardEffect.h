#pragma once

#include "xDefine.h"
#include "xSingleton.h"
#include "PveCardConfig.h"
#include "xSceneEntry.h"

using std::map;
class DScene;

//****************************************************************************
//*********************PveCardTarget******************************************
//****************************************************************************
class PveCardTargetBase
{
  public:
    PveCardTargetBase(const SPveCardEffectCFG* pCFG);
    virtual ~PveCardTargetBase();

    virtual bool getTarget(DScene* pScene, xSceneEntrySet& targetSet) { return true; }
  protected:
    const SPveCardEffectCFG* m_pEffectCFG = nullptr;
};

// 按规则随机玩家
class PveCardTargetRandomUser : public PveCardTargetBase
{
  public:
    PveCardTargetRandomUser(const SPveCardEffectCFG* pCFG);
    virtual ~PveCardTargetRandomUser();

    virtual bool getTarget(DScene* pScene, xSceneEntrySet& targetSet);
};

// 地图所有玩家
class PveCardTargetAllUser : public PveCardTargetBase
{
  public:
    PveCardTargetAllUser(const SPveCardEffectCFG* pCFG);
    virtual ~PveCardTargetAllUser();

    virtual bool getTarget(DScene* pScene, xSceneEntrySet& targetSet);
};

//****************************************************************************
//*********************PveCardEffect******************************************
//****************************************************************************
class PveCardBase
{
  public:
    PveCardBase(const SPveCardEffectCFG* pCFG, PveCardTargetBase* pTarget);
    virtual ~PveCardBase();

    virtual bool doEffect(DScene* pScene) = 0;

  protected:
    const SPveCardEffectCFG* m_pEffectCFG = nullptr;
    PveCardTargetBase* m_pTarget = nullptr;
};

// 招怪
class PveCardSummon : public PveCardBase
{
  public:
    PveCardSummon(const SPveCardEffectCFG* pCFG, PveCardTargetBase* pTarget);
    virtual ~PveCardSummon();

    virtual bool doEffect(DScene* pScene);
};

// pve 专用招怪
class PveCardPveSummon : public PveCardBase
{
  public:
    PveCardPveSummon(const SPveCardEffectCFG* pCFG, PveCardTargetBase* pTarget);
    virtual ~PveCardPveSummon();

    virtual bool doEffect(DScene* pScene);
};

// 加buff
class PveCardAddBuff : public PveCardBase
{
  public:
    PveCardAddBuff(const SPveCardEffectCFG* pCFG, PveCardTargetBase* pTarget);
    virtual ~PveCardAddBuff();

    virtual bool doEffect(DScene* pScene);
};

// 玩家or怪物 gm
class PveCardGM : public PveCardBase
{
  public:
    PveCardGM(const SPveCardEffectCFG* pCFG, PveCardTargetBase* pTarget);
    virtual ~PveCardGM();

    virtual bool doEffect(DScene* pScene);
};

// 场景gm
class PveCardSceneGM : public PveCardBase
{
  public:
    PveCardSceneGM(const SPveCardEffectCFG* pCFG, PveCardTargetBase* pTarget);
    virtual ~PveCardSceneGM();

    virtual bool doEffect(DScene* pScene);
};


typedef map<DWORD, PveCardBase*> TMapPveCardEffect;
class PveCardManager: public xSingleton<PveCardManager>
{
  friend class xSingleton<PveCardManager>;
  private:
    PveCardManager();
  public:
    virtual ~PveCardManager();

    bool init();
    PveCardBase* getCardEffect(DWORD cardid);

  private:
    void clear();
    PveCardTargetBase* createTargetObj(const SPveCardEffectCFG* pCFG);
    PveCardBase* createEffectObj(const SPveCardEffectCFG* pCFG, PveCardTargetBase* pTarget);
  private:
    TMapPveCardEffect m_mapPveCardEffects;
};

