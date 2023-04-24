#pragma once

#include "xSingleton.h"
#include "xLuaTable.h"

using std::map;

enum EWeddingServiceType
{
  EWEDDINGSERVICE_MIN = 0,
  EWEDDINGSERVICE_PLAN = 1,
  EWEDDINGSERVICE_PACKAGE = 2,
  EWEDDINGSERVICE_MAX = 3,
};

enum EWeddingPlanType
{
  EWEDDINGPLANTYPE_BLESS = 101, // 免费时间线祝福
  EWEDDINGPLANTYPE_FIREWORDS = 102, // 婚礼豪华礼花
  EWEDDINGPLANTYPE_CRUISE = 103, // 婚车环城巡游
  EWEDDINGPLANTYPE_SKY = 104, // 专属婚礼天空
  EWEDDINGPLANTYPE_BGM = 105, // 专属音乐bgm
};

struct SWeddingServiceCFG
{
  DWORD dwID = 0;
  EWeddingServiceType eType = EWEDDINGSERVICE_PLAN;
  string strName;
  map<DWORD, DWORD> mapPrice;
  TSetDWORD setServiceID;
  std::vector<xLuaData> vecEffectGMData;
  std::vector<xLuaData> vecSuccessGMData;
  TSetDWORD setSubPackageID;

  bool canBuy() const
  {
    return eType == EWEDDINGSERVICE_PACKAGE;
  }
  bool hasPlan(EWeddingPlanType type) const
  {
    if (eType == EWEDDINGSERVICE_PLAN)
      return dwID == static_cast<DWORD>(type);
    else if (eType == EWEDDINGSERVICE_PACKAGE)
      return setServiceID.find(static_cast<DWORD>(type)) != setServiceID.end();
    return false;
  }
  DWORD getPrice(DWORD itemid) const
  {
    auto it = mapPrice.find(itemid);
    if (it == mapPrice.end())
      return 0;
    return it->second;
  }
};

//駁獰饜离
struct SWeddingCFG 
{
  DWORD dwId = 0;
  DWORD m_dwStartHour = 0;
  DWORD m_dwEndHour = 0;
  DWORD m_dwPrice = 0;
};

typedef map<DWORD, SWeddingCFG> TMapId2WeddingCFG;

class WeddingConfig : public xSingleton<WeddingConfig>
{
  friend class xSingleton<WeddingConfig>;
private:
  WeddingConfig();
public:
  virtual ~WeddingConfig();

  bool loadConfig();
  bool checkConfig();

  bool loadWeddingCFG();
  bool loadWeddingService();

  const SWeddingServiceCFG* getWeddingServiceCFG(DWORD id);
  const SWeddingCFG* getWeddingCFG(DWORD id)
  {
    auto it = m_mapId2Wedding.find(id);
    return it == m_mapId2Wedding.end() ? nullptr : &(it->second);
  }
  bool isPackageFree(DWORD id);

  TMapId2WeddingCFG& getAllWeddingCFG() { return m_mapId2Wedding; }
  const TSetDWORD& getFreeWeddingPackage() { return m_setFreePackageID; }
  bool getPackagePrice(DWORD& price, DWORD itemid, DWORD packageid, const TSetDWORD& gotpackages = TSetDWORD());

  static string getSign(QWORD qwCharid, DWORD dwDate, DWORD dwConfigId, DWORD dwTime, bool bTicket, DWORD dwZoneId);
  static DWORD getUnixTime(DWORD dwDate, DWORD hour);

private:
  map<DWORD, SWeddingServiceCFG> m_mapID2Service;
  TMapId2WeddingCFG m_mapId2Wedding;
  TSetDWORD m_setFreePackageID;
};
