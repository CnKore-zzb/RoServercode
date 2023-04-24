/**
 * @file MusicBoxManager.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2016-03-09
 */

#pragma once

#include "xSingleton.h"
#include "SceneUser2.pb.h"

using namespace Cmd;

struct SMusicBase;
class SceneUser;
class SceneNpc;
class xRecord;

using std::vector;
using std::map;
using std::string;

const DWORD MAX_MUSIC_LIST_COUNT = 20;
const DWORD MUSIC_LOVE_FOREVER = 4004;

// music item
struct SMusicItem
{
  QWORD qwCharID = 0;
  QWORD qwNpcID = 0;

  DWORD dwNpcID = 0;
  DWORD dwMapID = 0;
  DWORD dwMusicID = 0;
  DWORD dwDemandTime = 0;
  DWORD dwStartTime = 0;
  DWORD dwEndTime = 0;

  DWORD dwState = 0;  // 0 : normal 1 : delete

  string strName;
  const SMusicBase* pBase = nullptr;

  SMusicItem() {}

  void toData(MusicItem* pData);
};
typedef vector<SMusicItem> TVecMusicItem;
typedef map<DWORD, TVecMusicItem> TMapMusicItem;

// music manager
class MusicBoxManager : public xSingleton<MusicBoxManager>
{
  friend class xSingleton<MusicBoxManager>;
  private:
    MusicBoxManager();
  public:
    virtual ~MusicBoxManager();

    bool init();

    void timer(DWORD dwCurTime);

    bool queryMusicList(SceneUser* pUser, QWORD qwNpcID, bool bSync = false);
    bool demandMusic(SceneUser* pUser, QWORD qwNpcID, DWORD dwMusicID);
    void initMusicNpc(SceneNpc* pNpc);
    void initActivityMusicNpc(SceneNpc* npc);

  private:
    bool loadMusicItem();
    void broadcastMusicList(QWORD qwNpcID, bool bPlay);
  private:
    TMapMusicItem m_mapMusicItem;
};

