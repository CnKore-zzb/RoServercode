#pragma once

#include "xTools.h"
#include "xSingleton.h"
#include "xLuaTable.h"
#include "PveCard.pb.h"

using namespace Cmd;
using std::map;
using std::vector;

struct SCardRecordData
{
  TVecDWORD vecBossID;//唯一boss卡放在vec第一个
  TVecDWORD vecGroupID;//小怪组合ID，不是卡牌ID
  TVecDWORD vecMonsterID; //普通小怪卡ID
  TVecDWORD vecItemID; // 道具卡ID
  TVecDWORD vecEnvID; // 环境卡ID

  TVecDWORD vecGroupMonsterID; // 小怪组合对应的小怪卡ID

  TVecDWORD vecAllCardIDs; // 记录以上所有的卡片ID
};

struct SPveRaidCFG
{
  DWORD dwID = 0;
  DWORD dwRaidID = 0;
  DWORD dwMonsterNum = 0; //出怪个数
  DWORD dwDifficulty = 0; // 副本难度系数
  TSetDWORD setRewardID; //通关奖励

  // dynamic data: cards info
  vector<SCardRecordData> vecCardInfo;
};
typedef map<DWORD, SPveRaidCFG> TMapPveRaidCFG;

enum EPveCardType
{
  EPVECARDTYPE_MIN = 0,
  EPVECARDTYPE_MONSTER = 1, //普通怪
  EPVECARDTYPE_BOSS = 2, //boss
  EPVECARDTYPE_ENV = 3, //环境卡
  EPVECARDTYPE_ITEM = 4, //道具卡
  EPVECARDTYPE_FRIEND = 5, // 友方牌
  EPVECARDTYPE_MAX,
};

struct SPveCardCFG
{
  DWORD dwCardID = 0;
  EPveCardType eType = EPVECARDTYPE_MIN;

  TVecDWORD vecEffectIDs; //卡牌效果, 对应PveCardEffect表ID
  TSetDWORD setDifficulty; //支持的难度系数
};
typedef map<DWORD, SPveCardCFG> TMapPveCardCFG;

typedef map<EPveCardType, map<DWORD, TSetDWORD>> TMapCardType2IDs; //<cardtype, <difficulty, setids>>
typedef map<EPveCardType, TSetDWORD> TMapCardType2AllIDs; // <cardtype, setids>

struct SPveCardGrpCFG
{
  DWORD dwGroupID = 0;
  TVecDWORD vecCardIDs;
  TSetDWORD setDifficulty; //支持的难度系数
};
typedef map<DWORD, SPveCardGrpCFG> TMapCardGrpCFG;

enum EPveCardTargetType
{
  EPVECARDTARGET_MIN = 0,
  EPVECARDTARGET_RANDOMUSER = 1,
  EPVECARDTARGET_ALLUSER = 2,
  EPVECARDTARGET_MAX,
};

enum EPveCardEffectType
{
  EPVECARDEFFECT_MIN = 0,
  EPVECARDEFFECT_GM = 1,
  EPVECARDEFFECT_SCENEGM = 2,
  EPVECARDEFFECT_SUMMON = 3,
  EPVECARDEFFECT_ADDBUFF = 4,
  EPVECARDEFFECT_PVESUMMON = 5,
  EPVECAREFFEECT_MAX,
};

struct SPveCardEffectCFG
{
  DWORD dwEffectID = 0;

  EPveCardTargetType eTargetType = EPVECARDTARGET_MIN;
  xLuaData oTargetParam;

  EPveCardEffectType eEffectType = EPVECARDEFFECT_MIN;
  xLuaData oEffectParam;
};
typedef map<DWORD, SPveCardEffectCFG> TMapPveCardEffectCFG;

class PveCardConfig : public xSingleton<PveCardConfig>
{
  friend class xSingleton<PveCardConfig>;
  private:
    PveCardConfig();
  public:
    virtual ~PveCardConfig();

  public:
    bool loadConfig();
    bool checkConfig();
  public:
    const TMapPveCardEffectCFG& getCardEffects() const { return m_mapPveCardEffectCFG; }
    const SPveRaidCFG* getPveRaidCFGByID(DWORD configid) const;
    const SPveCardCFG* getPveCardCFG(DWORD cardid) const;

    bool shuffleCard(DWORD pveRaidIndex, DWORD index, TVecDWORD& vecCardIDs) const;
    const TSetDWORD& getAllCardByType(EPveCardType eType) const;
    void randSystemCard(bool bIgnoreTime = false);
    void formatCardInfo(QueryCardInfoCmd& cmd) const;
    DWORD getNextCreateTime() const { return m_dwNextCreatTime; }
  private:
    bool loadPveRaidConfig();
    bool loadPveCardConfig();
    bool loadPveCardEffectConfig();
    bool loadPveCardGroupConfig();

    EPveCardType getCardType(const string& sType) const;
    EPveCardTargetType getCardTargetType(const string& sType) const;
    EPveCardEffectType getCardEffectType(const string& sType) const;

    void getIDsByTypeAndDif(EPveCardType eType, DWORD difficulty, TSetDWORD& ids);
  private:
    TMapPveRaidCFG m_mapPveRaidCFG;//pve raid

    TMapPveCardCFG m_mapPveCardCFG; // pve card
    TMapCardType2IDs m_mapPveCardType2IDs; // card type-> <difficulty, setids>
    TMapCardType2AllIDs m_mapPveCardType2AllIDs; // cardtype, setids

    TMapPveCardEffectCFG m_mapPveCardEffectCFG; // card effect

    TMapCardGrpCFG m_mapCardGrpCFG; // card group
    map<DWORD, TSetDWORD> m_mapPveGrpDif2Croups; // difficulty -> card groupids

    DWORD m_dwNextCreatTime = 0;
};

