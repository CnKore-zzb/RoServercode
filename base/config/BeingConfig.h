#pragma once

#include "xSingleton.h"

using namespace Cmd;
using std::map;

struct SBeingCFG
{
  DWORD dwID = 0;
  string strName;
  map<DWORD, DWORD> mapSkillID2UnlockLv;
  DWORD dwStaticSkill = 0;
};

struct SBeingBaseLvCFG
{
  DWORD dwLv = 0;
  QWORD qwExp = 0;
  DWORD dwSkillPoint = 0;
};

class BeingConfig : public xSingleton<BeingConfig>
{
  friend class xSingleton<BeingConfig>;
private:
  BeingConfig();
public:
  virtual ~BeingConfig();

  bool loadConfig();
  bool checkConfig();

  const SBeingCFG* getBeingCFG(DWORD id) const;
  const map<DWORD, SBeingCFG>& getAllBeingCFG() const { return m_mapBeingCFG; }
  const SBeingBaseLvCFG* getBeingBaseLvCFG(DWORD lv) const;

private:
  bool loadBeingConfig();
  bool loadBeingBaseLvConfig();

  map<DWORD, SBeingCFG> m_mapBeingCFG;
  map<DWORD, SBeingBaseLvCFG> m_mapBeingBaseLvCFG;
};
