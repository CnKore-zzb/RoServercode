#pragma once

#include "xNoncopyable.h"
#include "GuildSCmd.pb.h"
#include "xDefine.h"

class Guild;
class GMember;

class GuildShopMgr : private xNoncopyable
{
public:
  GuildShopMgr(Guild* guild);
  ~GuildShopMgr();

  bool buyItem(GMember* member, DWORD shopid, DWORD count);

private:
  Guild* m_pGuild = nullptr;
};
