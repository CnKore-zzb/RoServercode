/**
 * @file DeadConfig.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2018-07-03
 */

#pragma once

#include "xSingleton.h"
#include "TableManager.h"

struct SDeadLvCFG : public SBaseCFG
{
  DWORD dwLv = 0;
  DWORD dwExp = 0;
};
typedef map<DWORD, SDeadLvCFG> TMapDeadLvCFG;

// config
class DeadConfig : public xSingleton<DeadConfig>
{
  friend class xSingleton<DeadConfig>;
  private:
    DeadConfig();
  public:
    virtual ~DeadConfig();

    bool loadConfig();
    bool checkConfig();

    const SDeadLvCFG* getDeadCFG(DWORD lv) const;
  private:
    TMapDeadLvCFG m_mapLvCFG;
};

