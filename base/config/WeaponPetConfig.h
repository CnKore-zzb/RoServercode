#pragma once

#include "xTools.h"
#include "xSingleton.h"
#include "ScenePet.pb.h"

using namespace Cmd;
using std::map;

struct SWeaponPetCFG
{
  DWORD dwConfigID = 0;
  DWORD dwMonsterID = 0;
  DWORD dwReliveTime = 0;
  DWORD dwFollowDis = 0;
  TSetDWORD setConflictIDs;
  map<EEmployType, DWORD> mapType2Money;
  DWORD dwReleatedNpcID = 0;
  DWORD dwDiscount = 0;

  string name;
};

typedef map<DWORD, SWeaponPetCFG> TMapWeaponPetCFG;

class WeaponPetConfig : public xSingleton<WeaponPetConfig>
{
  friend class xSingleton<WeaponPetConfig>;
  private:
    WeaponPetConfig();
  public:
    virtual ~WeaponPetConfig();

    bool loadConfig();
    const SWeaponPetCFG* getCFG(DWORD id) const;
  private:
    TMapWeaponPetCFG m_mapID2CFG;
};
