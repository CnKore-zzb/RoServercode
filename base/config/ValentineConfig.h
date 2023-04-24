#pragma once

#include "xSingleton.h"
#include "xLuaTable.h"
#include <map>

enum EValentineType
{
  EValentineType_First = 1,   //第一题
  EValentineType_Next = 2,    
  EValentineType_End = 3,     //最终问题
};

struct SValentineCFG
{
public:
  bool isEnd() const { return m_type == EValentineType_End; } 
  bool isAll() const
  { 
    if (m_type == EValentineType_End ) 
      return true;
    else return false;
  }
  const SValentineCFG* getNextOption(DWORD a1, DWORD a2) const ;

  DWORD m_dwId = 0;           //
  TVecDWORD m_vecOption;
  EValentineType m_type;      
  string m_effect;
  string m_action;
  DWORD m_auguryType = 0;
  TVecDWORD m_vecReward;
  DWORD m_dwMonth = 0;
  SValentineCFG() {}
};

class ValentineConfig : public xSingleton<ValentineConfig>
{
  public:
    ValentineConfig();
    virtual ~ValentineConfig();

    bool loadConfig();
    bool loadSubConfig(DWORD type, const string& tableName, DWORD month);
    bool checkConfig();
    const SValentineCFG* getFirstTitleCfg(DWORD type, DWORD month) ;
    const SValentineCFG* getCfg(DWORD type, DWORD month, DWORD id) ;
  private:
    bool monthTable(DWORD type);
    
    std::map<DWORD/*type*/, std::map<DWORD/*month*/, std::map<DWORD/*id*/, SValentineCFG>>> m_mapValentineCFG;
    std::map<DWORD/*type*/ ,std::map<DWORD/*month*/, std::vector<DWORD>>> m_mapVecFirst;
};

