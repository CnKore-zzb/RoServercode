/**
 * @file GuildQuest.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2018-01-08
 */

#pragma once

#include "xNoncopyable.h"
#include "xDefine.h"
#include "GuildSCmd.pb.h"

using namespace Cmd;
using std::map;

class Guild;
class GMember;
class GuildQuestMgr : private xNoncopyable
{
  public:
    GuildQuestMgr(Guild* pGuild);
    virtual ~GuildQuestMgr();

    void fromData(const BlobGQuest& quest);
    void toData(BlobGQuest* quest);
    void toData(QueryGQuestGuildCmd& cmd);

    bool isSubmit(DWORD dwQuestID) const { return m_setSubmitIDs.find(dwQuestID) != m_setSubmitIDs.end(); }
    const TSetDWORD& getSubmitList() const { return m_setSubmitIDs; }

    void questSyncToZone(QWORD charid = 0);
    void updateQuest(QWORD charid, DWORD questid);
  private:
    Guild* m_pGuild = nullptr;

    TSetDWORD m_setSubmitIDs;
};

