/**
 * @file GuildPhoto.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2017-07-15
 */
#pragma once

#include <list>
#include "xNoncopyable.h"
#include "xDefine.h"
#include "GameStruct.h"
#include "GuildConfig.h"
#include "GuildSCmd.pb.h"

class Guild;

using std::map;
using std::list;
using std::string;
using namespace Cmd;

const DWORD MAX_FRAME_OFFLINE_LOADCOUNT = 3;
typedef map<string, GuildPhoto> TMapIDPhoto;
typedef map<DWORD, TMapIDPhoto> TMapFramePhoto;

class GuildPhotoMgr : private xNoncopyable
{
  public:
    GuildPhotoMgr(Guild *pGuild);
    virtual ~GuildPhotoMgr();

    void init();

    void setBlobPhotoString(const char* str, DWORD len);
    bool toBlobPhotoString(string& str);

    void queryPhotoList();
    void queryUserPhotoList(const SocialUser& rUser);
    void queryAutoPhoto(QWORD qwAccID = 0, DWORD dwZoneID = 0, EPhotoAction eAction = EPHOTOACTION_LOAD_FROM_RECORD);
    void updateFrame(const FrameUpdateGuildSCmd& cmd);
    void updatePhoto(const PhotoUpdateGuildSCmd& cmd);
    void refreshPhoto();

  private:
    bool fromPhotoString(const string& str);

    void addFramePhoto(DWORD dwFrameID, const GuildPhoto& rPhoto);
    void removeFramePhoto(DWORD dwFrameID, const GuildPhoto& rPhoto);
  private:
    Guild *m_pGuild = nullptr;

    DWORD m_dwInitStatus = GUILD_BLOB_INIT_NULL;
    string m_oBlobPhoto;

    TMapFramePhoto m_mapFramePhoto;
};

