#pragma once

#include "xNoncopyable.h"
#include "xDefine.h"
#include "xLuaTable.h"
#include <map>

using std::string;
using std::map;

class TradePriceMgr :public xSingleton<TradePriceMgr>
{
  public:
    TradePriceMgr();
    virtual ~TradePriceMgr();
    bool get();
    bool getPrice(std::map<DWORD/*itemid*/, QWORD/*price*/>& mapItem);
    QWORD getPrice(DWORD itemId);
   
  private:
    DWORD m_dwRegionID = 0;
    std::string m_strRedisKey;
    xLuaData m_oData;
};
