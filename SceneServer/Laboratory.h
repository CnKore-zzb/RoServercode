#pragma once

#include <set>
#include <list>
#include "xNoncopyable.h"
#include "xDefine.h"

// config data
struct LaboratoryNpc
{
  DWORD m_dwGroupID = 0;
  DWORD m_dwNpcID = 0;
  DWORD m_dwPoint = 0;
};

struct LaboratoryConfig
{
  MinMax level;

  // MonsterGroup Monster
  std::map<DWORD, std::list<LaboratoryNpc>> npclist;
};

// config
class Laboratory : private xNoncopyable
{
  public:
    Laboratory();
    virtual ~Laboratory();
  public:
    static bool loadConfig();
    static std::map<DWORD, LaboratoryConfig> s_cfg;
};
