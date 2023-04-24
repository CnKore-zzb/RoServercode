#pragma once

#include "xNoncopyable.h"
#include "xDefine.h"
#include "GameStruct.h"
#include "GuildCmd.pb.h"
#include <list>

class Guild;
using namespace Cmd;

// guild event
enum EGuildEvent
{
  EGUILDEVENT_MIN = 0,
  EGUILDEVENT_CREATEGUILD = 1,
  EGUILDEVENT_SET_DISMISSGUILD = 2,
  EGUILDEVENT_CANCEL_DISMISSGUILD = 3,
  EGUILDEVENT_EDIT_BOARD = 4,
  EGUILDEVENT_EDIT_RECRUIT = 5,
  EGUILDEVENT_LEADER_CHANGE = 6,
  EGUILDEVENT_VICE_SET = 7,
  EGUILDEVENT_KICK_MEMBER = 8,
  EGUILDEVENT_JOBNAME_CHANGE = 9,
  EGUILDEVENT_NEW_MEMBER = 10,
  EGUILDEVENT_EXIT_MEMBER = 11,
  EGUILDEVENT_DONATE = 12,
  EGUILDEVENT_AUTH_ADD = 13,
  EGUILDEVENT_AUTH_REMOVE = 14,
  EGUILDEVENT_EDITAUTH_ADD = 15,
  EGUILDEVENT_EDITAUTH_REMOVE = 16,
  EGUILDEVENT_RENAME = 17,
  EGUILDEVENT_BUILDING_BUILD = 18,
  EGUILDEVENT_BUILDING_LVUP = 19,
  EGUILDEVENT_ARTIFACT_PRODUCE = 20,
  EGUILDEVENT_ARTIFACT_DISTRIBUTE = 21,
  EGUILDEVENT_ARTIFACT_GIVEBACK = 22,
  EGUILDEVENT_ARTIFACT_RETRIEVE = 23,
  EGUILDEVENT_TREASURE_OPEN_GVG = 24,
  EGUILDEVENT_TREASURE_OPEN_GUILD = 25,
  EGUILDEVENT_BUILDING_SUPPLY = 27,
  EGUILDEVENT_GVG_GET = 28,
  EGUILDEVENT_GVG_LOST = 29,
  EGUILDEVENT_SUPERGVG_RANK = 30,
  EGUILDEVENT_ARTIFACT_UNLOCK = 31,
  EGUILDEVENT_OPEN_REALTIME_VOICE = 32,
  EGUILDEVENT_CLOSE_REALTIME_VOICE = 33,
  EGUILDEVENT_MAX,
};

typedef std::list<GuildEvent> TListGuildEvent;

class GuildEventM : private xNoncopyable
{
  public:
    GuildEventM(Guild *pGuild);
    virtual ~GuildEventM();

  public:
    const GuildEvent* addEvent(EGuildEvent eType, const TVecString& vecParams);
    const GuildEvent* getEvent(DWORD dwID);
    void collectEventList(QueryEventListGuildCmd& cmd);
  public:
    bool toBlobEventString(std::string& str);

  public:
    void init();
  private:
    DWORD m_dwInitStatus = GUILD_BLOB_INIT_NULL;

    // 璁剧疆misc str
  public:
    void setBlobEventString(const char* str, DWORD len);
  private:
    std::string m_oBlobEvent;

  private:
    bool fromEventString(const std::string& str);

  private:
    TListGuildEvent m_listGuildEvent;
  private:
    Guild *m_pGuild = nullptr;
};
