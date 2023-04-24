/**
 * @file GuildPhotoLoader.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2018-04-14
 */

#pragma once

#include <list>
#include "GuildSCmd.pb.h"
#include "xSingleton.h"

struct SPhotoResult
{
  DWORD dwFrameID = 0;
  DWORD dwNeedCount = 0;

  std::list<Cmd::GuildPhoto> listPhotos;
};
struct SPhotoLoad
{
  QWORD qwGuildID = 0;

  TSetQWORD setMembers;
  TSetString setExists;

  std::map<DWORD, SPhotoResult> mapResult;

  bool isComplete();
};
typedef std::map<QWORD, SPhotoLoad> TMapLoadList;

class GuildPhotoLoader : public xSingleton<GuildPhotoLoader>
{
  friend class xSingleton<GuildPhotoLoader>;
  private:
    GuildPhotoLoader();
  public:
    virtual ~GuildPhotoLoader();

    bool addLoad(const Cmd::QueryShowPhotoGuildSCmd& cmd);
    void timer(DWORD curSec);
  private:
    void processLoad();
    void syncLoad(QWORD qwID);
  private:
    TMapLoadList m_mapLoad;
};

