#pragma once

#include "xDefine.h"
#include "RecordCmd.pb.h"
#include "AstrolabeCmd.pb.h"
#include "AstrolabeConfig.h"
#include "MiscConfig.h"

using namespace Cmd;
using std::map;

inline EAstrolabeType getAstrolabeType(DWORD id)
{
  return EASTROLABETYPE_PROFESSION;
}

typedef map<DWORD, DWORD> TMapDWORD;

class SceneUser;

// 星盘星位
struct SAstrolabeStar
{
  DWORD dwId = 0;
};

typedef map<DWORD, SAstrolabeStar> TMapAstrolabeStar;

class Astrolabe;
typedef map<DWORD, Astrolabe> TMapAstrolabe;
typedef map<EAstrolabeType, TMapAstrolabe> TMapType2Astrolabe;

class Astrolabes
{
public:
  Astrolabes(SceneUser* pUser);
  ~Astrolabes();

  bool load(const BlobAstrolabe& rData);
  bool save(BlobAstrolabe* pBlob);

  // 加载多职业
  bool loadProfessionData(const BlobAstrolabe& rData, const std::vector<std::pair<DWORD, DWORD>>& ids, bool& isReset, bool isNeedReset = false);

  Astrolabe* getAstrolabe(DWORD id);
  DWORD getTypeBranch();
  void onProfesChange(EProfession oldProfes);

  /* 特殊效果 */
  void chgEffectCnt(EAstrolabeType type, DWORD specId, SDWORD cnt);
  DWORD getEffectCnt(DWORD specId);
  bool isEffectUnlock(DWORD specId) { return getEffectCnt(specId) > 0; } // 特殊效果是否解锁
  void getEffectIDs(TSetDWORD& set);
  void collectRuneCount(DWORD& dwTotal, DWORD& dwSpecial);

  /* 激活 */
  bool isActive(DWORD id) { return getAstrolabe(id) != nullptr; }
  bool isStarActive(DWORD id, DWORD starid);
  bool canActivate(DWORD id);
  bool activate(DWORD id); // 激活星盘
  bool activateStar(DWORD id, DWORD starid);
  bool activateStar(const vector<pair<DWORD, DWORD>>& ids, bool isConnectCheck = true, bool isCostItems = true);
  //void collectAttr(TVecAttrSvrs& attrs);
  void collectAttr();
  bool activateStarForce(DWORD id, DWORD starid);
  bool delStarForce(DWORD id, DWORD starid);

  /* 重置 */
  /* void resetForce(EAstrolabeType type); */
  bool reset(set<pair<DWORD, DWORD>>& ids, bool free = false, bool isReturnItems = true);
  bool resetAll(bool free = false, bool isReturnItems = true);
  bool resetAll(std::vector<std::pair<DWORD, DWORD>>& ids); // 返回列表，并不直接返回物品，多职业切换用

  /* 消息处理 */
  void sendAstrolabeData();
  void handleActivateStar(AstrolabeActivateStarCmd& rev);
  void handleReset(AstrolabeResetCmd& rev);

  DWORD getPetAdventureScore();
  
public:
  bool changeCost(const vector<pair<DWORD, DWORD>>& ids_old, vector<pair<DWORD, DWORD>>& ids_new);
  bool getCost(const vector<pair<DWORD, DWORD>>& ids, TVecItemInfo& cost);
  bool getTotalCost(TVecItemInfo& cost);
  void ntfReset();

private:
  bool create(DWORD id);
  bool canConnectInitStar(DWORD id, DWORD starid, DWORD ignoreId, DWORD ignoreStarId)
  {
    if (MiscConfig::getMe().getAstrolabeCFG().isInitStar(getAstrolabeType(ignoreId), ignoreId, ignoreStarId))
      return false;
    set<DWORD> ignores;
    ignores.insert(ignoreId * 10000 + ignoreStarId);
    return canConnectInitStar(id, starid, ignores);
  }
  bool canConnectInitStar(DWORD id, DWORD starid, set<DWORD>& ignores);

  TMapType2Astrolabe m_mapType2Astr;
  map<EAstrolabeType, TMapDWORD> m_mapType2EffectCnt;
  SceneUser* m_pUser = nullptr;
};


// 星盘
class Astrolabe
{
  friend Astrolabes;
public:
  Astrolabe(SceneUser* pUser, DWORD id);
  ~Astrolabe();

  bool load(const AstrolabeData& rData);
  bool save(AstrolabeData* pData);
  bool isNeedSave();

  DWORD getId() { return m_dwId; }
  template<class T> void foreach(T func)
  {
    for_each(m_mapStar.begin(), m_mapStar.end(), [func](TMapAstrolabeStar::value_type& r) { func(r.second); });
  }

  bool isStarActive(DWORD id) { return m_mapStar.find(id) != m_mapStar.end(); }
  void activateStar(DWORD id, bool event); // 激活星位
  void delStar(DWORD id); // 删除星位
  //void collectAttr(TVecAttrSvrs& attrs);
  void collectAttr();
  void getIds(vector<pair<DWORD, DWORD>>& ids);

  const TMapAstrolabeStar& getStarList() const { return m_mapStar; }

  void copyFrom(Astrolabe& astr);
  private:
  DWORD m_dwId = 0;
  SceneUser* m_pUser = nullptr;
  TMapAstrolabeStar m_mapStar;
};
