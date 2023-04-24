#pragma once

#include "xSingleton.h"
#include "xLuaTable.h"

using std::map;
using std::string;
using std::vector;

struct SXoclientCFG
{
  DWORD m_dwId = 0;   //activity id
  DWORD m_dwType = 0;
  DWORD m_dwAnserId = 0;
  SXoclientCFG() {}
};

class XoclientConfig : public xSingleton<XoclientConfig>
{
  public:
    XoclientConfig();
    virtual ~XoclientConfig();

    bool loadConfig();
    bool loadServerConfig();

    const SXoclientCFG* getXoclientCFG(DWORD dwID) const;
    std::vector<SXoclientCFG*> getRandomXoclient(DWORD count);
    bool isActivityQuestion(DWORD dwId)
    { 
      auto it = m_mapxoclientCFG.find(dwId); 
      if (it != m_mapxoclientCFG.end() && it->second.m_dwType == 2)
        return true; 
      else
        return false;
    }
  private:
    map<DWORD, SXoclientCFG> m_mapxoclientCFG;
    std::vector<SXoclientCFG*> m_vecClientCfg;      //type =2
};

