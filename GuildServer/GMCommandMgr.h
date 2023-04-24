#pragma once

#include "xSingleton.h"
#include "xLuaTable.h"
#include "GuildSCmd.pb.h"
#include "xDefine.h"

using namespace Cmd;
using std::vector;
using std::string;

class Guild;

class GMCommandMgr : public xSingleton<GMCommandMgr>
{
public:
  GMCommandMgr() {}
  ~GMCommandMgr() {}

  bool exec(const GMCommandGuildSCmd& cmd);
private:
  Guild* getGuild(const GuildGM& info);

  bool building(const GuildGM& info, const xLuaData& data);
  bool challenge(const GuildGM& info, const xLuaData& data);
  bool additem(const GuildGM& info, const xLuaData& data);
  bool reduceitem(const GuildGM& info, const xLuaData& data);
  bool showItem(const GuildGM& info, const xLuaData& data);
  bool artifact(const GuildGM& info, const xLuaData& data);
  bool quest(const GuildGM& info, const xLuaData& data);
  bool addcon(const GuildGM& info, const xLuaData& data);
  bool subcon(const GuildGM& info, const xLuaData& data);
  bool gvg(const GuildGM& info, const xLuaData& data);
  bool addasset(const GuildGM& info, const xLuaData& data);
  bool reward(const GuildGM& info, const xLuaData& data);
  bool setvar(const GuildGM& info, const xLuaData& data);
  bool auth(const GuildGM& info, const xLuaData& data);
  bool iconstate(const GuildGM& info, const xLuaData& data);
};
