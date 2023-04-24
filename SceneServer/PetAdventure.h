/**
 * @file PetAdventure.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2017-08-09
 */

#pragma once

#include "RecordCmd.pb.h"
#include "xDefine.h"
#include "PetConfig.h"

using namespace Cmd;
using std::map;

const DWORD REMOVE_STEP_VERSION = 1;
const DWORD MAX_VERSION = 2;

struct SPetAdventureCFG;
class SceneUser;
class BuffPetAdventure;
class ItemEgg;
class OtherFactor;

class PetAdventure
{
  public:
    PetAdventure(SceneUser* pUser);
    ~PetAdventure();

    static bool toData(PetAdventureDBItem& rDBItem, const PetAdventureItem& rItem);
    static bool fromData(PetAdventureItem& rItem, const PetAdventureDBItem& rDBItem);
    static bool calcPreview(SceneUser* pUser, PetAdventureItem& rItem, const SPetAdventureCFG* pCFG);
    static bool calcReward(SceneUser* pUser, PetAdventureItem& rItem, const SPetAdventureCFG* pCFG, DWORD dwRareCount, float fEffectValue, const OtherFactor& factor, float fRatio = 1.0f);
    static bool collectMonsterID(const SPetAdventureCFG* pCFG, TVecDWORD& vecResult, DWORD& dwKind, DWORD specMonsterID);

    bool load(const BlobPetAdventure& rBlob);
    bool save(BlobPetAdventure* pBlob);

    bool isComplete(DWORD dwID) const;
    bool hasPet(DWORD dwPetID, DWORD dwBaseLv, DWORD dwFriendLv) const;

    void sendAdventureList();

    bool startAdventure(const StartAdventurePetCmd& cmd);
    bool getAdventureReward(const GetAdventureRewardPetCmd& cmd);

    void timer(DWORD curSec);
  private:
    bool isAreaUnlock(DWORD dwID) const { return m_setUnlockArea.find(dwID) != m_setUnlockArea.end(); }
    float calcScore(EPetEfficiencyType eType);
    void patch_1();
  private:
    SceneUser* m_pUser = nullptr;

    map<DWORD, PetAdventureItem> m_mapIDAdventure;
    TSetDWORD m_setUnlockArea;

    DWORD m_dwVersion = 0;
  public:
    /**
     * 计算冒险Buffer带来的额外增益
     * @param rItem 原来的奖励
     */ 
    void calcBuffReward(const SPetAdventureCFG* pCFG, PetAdventureItem& rItem);
    /**
     * 检查并收集宠物身上和冒险有关的Buff 
     */
    void checkAdventureBuffs(ItemEgg* pEgg);
    /**
     * 返回不消耗丸子的概率
     */ 
    DWORD getNotConsumeRatio();
    /**
     * 返回战斗时间减少的比例
     */ 
    float getReduceBattleTimeRatio();
    /**
     * 返回减少消耗时间的比例
     */ 
    float getReduceConsumeTimeRatio();
    /**
     * 返回物品随机增加的概率
     */ 
    float getRewardKindRatio();
    /**
     * 返回卡牌的概率
     */ 
    float getCardRatio();
    //稀有箱子增加概率 
    DWORD getRareBoxRatio();
  private:
    typedef std::vector<BuffPetAdventure*> VecAdventureBuff;
    std::vector<VecAdventureBuff> m_vecPetAdventureBuff;
};

