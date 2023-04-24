/**
 * @file GuildTreasure.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2018-01-26
 */

#pragma once

#include "GuildRaidConfig.h"
#include "xNoncopyable.h"
#include "GuildSCmd.pb.h"

typedef std::map<DWORD, Cmd::TreasureResult> TMapTreasureResult;

class Guild;
class GTreasure : private xNoncopyable
{
  public:
    GTreasure(Guild* pGuild);
    virtual ~GTreasure();

    bool fromData(const Cmd::BlobGuildTreasure& data);
    bool toData(Cmd::BlobGuildTreasure* pData);
    bool hasTreasure() const { return m_mapTreasure.empty() == false; }

    void queryTreasureToScene();
    void queryTreasureResult(QWORD charid, DWORD dwEventGUID);

    bool win(DWORD dwCityID, DWORD dwTimes);
    bool open(QWORD charid, DWORD treasureid);
    bool addResult(const TreasureResultNtfGuildSCmd& cmd);
  private:
    Guild* m_pGuild = nullptr;
    TMapTreasure m_mapTreasure;
    TMapTreasureResult m_mapTreasureResult;
};

