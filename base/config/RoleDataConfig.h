#pragma once

#include "xSingleton.h"
#include "xEntryManager.h"

struct RoleData : public xEntry
{
  virtual ~RoleData() {}

  bool bPercent = false;
  bool bSync = false;
  bool bFloatSync = false;
  std::string prop;
};

class RoleDataConfig : public xEntryManager<xEntryID,xEntryName>, public xSingleton<RoleDataConfig>
{
  friend class xSingleton<RoleDataConfig>;
  public:
    virtual ~RoleDataConfig();
  private:
    RoleDataConfig();
  public:
    const RoleData* getRoleData(DWORD id);

    DWORD getIDByName(const char *name);
    bool isPercent(const char* name);
    bool isPercent(DWORD id);
    bool needSyncPercent(DWORD id);

    bool isSync(const char* name);
    bool isSync(DWORD id);
  public:
    bool loadConfig();
};
