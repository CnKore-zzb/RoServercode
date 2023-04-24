#pragma once

#include "xSingleton.h"
#include "SceneUser2.pb.h"
#include "MusicBoxManager.h"
#include "RecordCmd.pb.h"

// music manager
class GuildMusicBoxManager : public xSingleton<GuildMusicBoxManager>
{
  friend class xSingleton<GuildMusicBoxManager>;
  private:
    GuildMusicBoxManager();
  public:
    virtual ~GuildMusicBoxManager();

    void sendGuildMusicQueryRecord(DWORD sceneid, QWORD guildid);
    bool loadMusicItem(const GuildMusicQueryRecordCmd& cmd);

    void timer(DWORD dwCurTime);

    bool queryMusicList(SceneUser* pUser, QWORD qwNpcID, bool bSync = false);
    bool demandMusic(SceneUser* pUser, QWORD qwNpcID, DWORD dwMusicID);
    void initMusicNpc(SceneNpc* pNpc);
    void onSceneClose(QWORD guildid);

  private:
    void broadcastMusicList(QWORD qwNpcID, bool bPlay);
  private:
    map<QWORD, TVecMusicItem> m_mapMusicItem;
};

