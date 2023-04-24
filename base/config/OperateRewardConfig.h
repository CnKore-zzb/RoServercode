#pragma once

#include "xSingleton.h"
#include "xLuaTable.h"

using std::map;
using std::string;
using std::vector;

struct SOperateRewardCFG
{
  DWORD dwId = 0;
  string uid;
  DWORD type = 0;
  DWORD dwRewardId = 0;
  DWORD dwZeny = 0;
  DWORD dwMonthcardCount = 0;
  SOperateRewardCFG() {}
};

class OperateRewardConfig : public xSingleton<OperateRewardConfig>
{
public:
  OperateRewardConfig();
  virtual ~OperateRewardConfig();

  bool loadConfig();

  SOperateRewardCFG* getOperateRewardCFG(string uid, DWORD type) ;
private:
  map<DWORD, SOperateRewardCFG> m_mapOperateRewardCfg;
};

