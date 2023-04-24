/**
 * @file GuildGmMgr.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2018-01-11
 */

#pragma once

#include "xSingleton.h"
#include "xDefine.h"
#include "GuildSCmd.pb.h"

using std::map;
using namespace Cmd;

struct SGuildGM
{
  DWORD dwCreateTime = 0;
  GMCommandGuildSCmd cmd;
};

typedef map<DWORD, SGuildGM> TMapSessionGM;

class GuildGmMgr : public xSingleton<GuildGmMgr>
{
  friend class xSingleton<GuildGmMgr>;
  private:
    GuildGmMgr();
  public:
    virtual ~GuildGmMgr();

    bool add(const GMCommandGuildSCmd& cmd);
    bool respond(const GMCommandRespondGuildSCmd& cmd);

    void timer(DWORD curSec);
  private:
    void flush(DWORD curSec);
  private:
    TMapSessionGM m_mapSessionGM;

    DWORD m_dwFlushTick = 0;
};

