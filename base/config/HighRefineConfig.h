#pragma once

#include "xSingleton.h"
#include <map>
#include <vector>
#include "xLuaTable.h"

//炼金合成
struct SMatCompose
{
  DWORD dwId = 0;
  DWORD dwGroupId = 0;
  DWORD dwProductId = 0;
  std::pair<DWORD/*min*/, DWORD/*max*/> prCount;    //[min, max]
  std::map<DWORD/*itemid*/, DWORD> mapMainMat;      //主料
  std::map<DWORD/*itemid*/, DWORD> mapViceMat;         //辅料
  bool checkPercent(bool main, TVecItemInfo vecItemInfo) const;
  DWORD randomCount() const;
  DWORD dwCostZeny = 0;
};

typedef std::map<DWORD/*id*/, SMatCompose> TMapId2MatCompose;

struct JobAttr
{
  std::vector<EProfession> vecJob;
  TVecAttrSvrs vecAttr;
  bool checkJob(EProfession job) const;
};

struct SHighRefine
{
  DWORD dwId = 0;
  EEquipPos ePos = EEQUIPPOS_MIN;
  DWORD dwType = 0;     //类型， 1 初级 2中级  3高级 4极限
  DWORD dwLevel = 0;
  DWORD dwPreLevel = 0;
  DWORD dwRefineLevel = 0;  //属性生效最低精炼等级
  std::vector<ItemInfo> oCost;
  std::vector<JobAttr> vecAttr;
};


// config
class HighRefineConfig : public xSingleton<HighRefineConfig>
{
  friend class xSingleton<HighRefineConfig>;
private:
  HighRefineConfig();
public:
  virtual ~HighRefineConfig();
  bool loadConfig();
  bool checkConfig();

  const SMatCompose* getMatComposeConfig(DWORD dwId) const
  {
    auto it = m_mapMatCompose.find(dwId);
    return it == m_mapMatCompose.end() ? nullptr : &(it->second);
  }

  const SHighRefine* getHighRefineConfig(DWORD dwId) const {
    auto it = m_mapHighRefine.find(dwId);
    return it == m_mapHighRefine.end() ? nullptr : &(it->second);
  }
  const SHighRefine* getHighRefineConfigByLv(EEquipPos pos, DWORD dwLv) const {
    auto it = m_mapHighRefineLv.find(getKey(pos, dwLv));
    return it == m_mapHighRefineLv.end() ? nullptr : getHighRefineConfig(it->second);
  }
  
  EEquipPos getValidPos(EEquipPos pos) const {
    auto it = m_mapPos2RealPos.find(pos);
    return it == m_mapPos2RealPos.end() ? pos : it->second;
  }

  DWORD getKey(EEquipPos pos, DWORD dwLv) const { return pos * 10000 + dwLv; }

private:
  bool loadMatConfig();
  bool loadHighRefineConfig();

private:
  std::map<DWORD, SMatCompose> m_mapMatCompose;
  std::map<DWORD, SHighRefine> m_mapHighRefine;
  std::map<DWORD/*pos*10000 + lv*/, DWORD/*id*/> m_mapHighRefineLv;
  std::map<EEquipPos, EEquipPos> m_mapPos2RealPos;;
};

