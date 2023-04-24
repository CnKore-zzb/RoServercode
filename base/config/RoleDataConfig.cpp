#include "RoleDataConfig.h"
#include "xLuaTable.h"

RoleDataConfig::RoleDataConfig()
{
}

RoleDataConfig::~RoleDataConfig()
{
  struct CallBack : public xEntryCallBack
  {
    virtual bool exec(xEntry* e)
    {
      if (e == nullptr)
        return false;
      RoleDataConfig::getMe().removeEntry(e);
      SAFE_DELETE(e);
      return true;
    }
  };
  CallBack oBack;
  forEach(oBack);
}

bool RoleDataConfig::loadConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_RoleData.txt"))
  {
    XERR << "[RoleData配置-加载] 加载表格 Table_RoleData.txt 失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_RoleData", table);

  for (auto it=table.begin(); it!=table.end(); ++it)
  {
    xLuaData &data = it->second;

    RoleData *rd = new RoleData();
    rd->set_id(it->first);
    rd->set_name(data.getTableString("VarName"));
    rd->prop = data.getTableString("PropName");
    rd->bPercent = data.getTableInt("IsPercent") == 1;
    rd->bSync = data.getTableInt("Sync") == 1;
    rd->bFloatSync = data.getTableInt("SyncFloat") == 1;
    if (!addEntry(rd))
    {
      XERR << "[RoleData配置-加载] 添加" << rd->id << rd->name << rd->prop << "失败" << XEND;
      SAFE_DELETE(rd);
      bCorrect = false;
    }
  }

  if (bCorrect)
    XLOG << "[RoleData配置-加载] 加载表格 Table_RoleData.txt 成功" << XEND;
  return bCorrect;
}

const RoleData* RoleDataConfig::getRoleData(DWORD id)
{
  return dynamic_cast<const RoleData*>(getEntryByID(id));
}

DWORD RoleDataConfig::getIDByName(const char *name)
{
  if (!name) return 0;
  xEntry *rd = getEntryByName(name);
  if (rd)
    return rd->id;

  return 0;
}

bool RoleDataConfig::isPercent(const char* name)
{
  const RoleData* pData = dynamic_cast<const RoleData*>(getEntryByName(name));
  if (pData == nullptr)
    return false;

  return pData->bPercent;
}

bool RoleDataConfig::isPercent(DWORD id)
{
  const RoleData* pData = dynamic_cast<const RoleData*>(getEntryByID(id));
  if (pData == nullptr)
    return false;

  return pData->bPercent;
}

bool RoleDataConfig::isSync(const char* name)
{
  const RoleData* pData = dynamic_cast<const RoleData*>(getEntryByName(name));
  if (pData == nullptr)
    return false;

  return pData->bSync;
}

bool RoleDataConfig::isSync(DWORD id)
{
  const RoleData* pData = dynamic_cast<const RoleData*>(getEntryByID(id));
  if (pData == nullptr)
    return false;

  return pData->bSync;
}

bool RoleDataConfig::needSyncPercent(DWORD id)
{
  const RoleData* pData = dynamic_cast<const RoleData*>(getEntryByID(id));
  if (pData == nullptr)
    return false;
  return pData->bPercent || pData->bFloatSync;
}

