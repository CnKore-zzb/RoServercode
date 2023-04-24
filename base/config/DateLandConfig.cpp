#include "DateLandConfig.h"
#include "ItemConfig.h"
#include "MapConfig.h"
#include "xLuaTable.h"

DateLandConfig::DateLandConfig()
{
}

DateLandConfig::~DateLandConfig()
{
}

bool DateLandConfig::loadConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_DateLand.txt")) {
    XERR << "[约会圣地配置] 加载配置Table_DateLand.txt失败" << XEND;
    return false;
  }

  m_mapDateLand.clear();

  bool bCorrect = true;
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_DateLand", table);
  for (auto m = table.begin(); m != table.end(); ++m) {
    DWORD id = m->second.getTableInt("id");

    if (m_mapDateLand.find(id) != m_mapDateLand.end()) {
      XERR << "[约会圣地配置] id:" << id << "重复了" << XEND;
      bCorrect = false;
      continue;
    }

    SDateLandCFG& rCfg = m_mapDateLand[id];
    rCfg.dwId = id;
    rCfg.sName = m->second.getTableString("Name");
    rCfg.dwInviteOverTime = m->second.getTableInt("invite_overtime");
    rCfg.dwInviteMaxCount = m->second.getTableInt("invite_maxcount");
    rCfg.dwEnterWaitTime = m->second.getTableInt("enter_waittime");
    rCfg.dwTicketItem = m->second.getTableInt("ticket_item");
    rCfg.dwRaidID = m->second.getTableInt("raidid");
    rCfg.dwCountDown = m->second.getTableInt("countdown");
    rCfg.bGender = m->second.getTableInt("gender");
  }

  if (bCorrect)
    XLOG << "[约会圣地配置] 成功加载Table_DateLand.txt" << XEND;
  return bCorrect;
}

bool DateLandConfig::checkConfig()
{
  bool bCorrect = true;
  for (auto &v : m_mapDateLand) {
    if (ItemConfig::getMe().getItemCFG(v.second.dwTicketItem) == nullptr) {
      XERR << "[约会圣地配置] id:" << v.second.dwId << "ticketitem:" << v.second.dwTicketItem << "未在 Table_Item.txt 表中找到" << XEND;
      bCorrect = false;
    }
    if (MapConfig::getMe().getRaidCFG(v.second.dwRaidID) == nullptr) {
      XERR << "[约会圣地配置] id:" << v.second.dwId << ":" << v.second.dwRaidID << "未在 Table_MapRaid.txt 表中找到" << XEND;
      bCorrect = false;
    }
  }

  return bCorrect;
}

const SDateLandCFG* DateLandConfig::getDateLandCFG(DWORD id) const
{
  auto it = m_mapDateLand.find(id);
  if (it != m_mapDateLand.end())
    return &it->second;
  return nullptr;
}
