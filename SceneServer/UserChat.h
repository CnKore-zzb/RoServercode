/**
 * @file UserChat.cpp
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2017-02-20
 */

#pragma once

#include <list>
#include "xDefine.h"
#include "RecordCmd.pb.h"

using std::list;
using std::map;
using namespace Cmd;

class SceneUser;

typedef list<ChatCount> TListChatCount;
typedef list<ChatItem> TListChatItem;

class UserChat
{
  public:
    UserChat(SceneUser* pUser);
    ~UserChat();

    bool load(const BlobChat& rChat);
    bool save(BlobChat* pChat);

    void onChat(const ChatRetCmd& cmd);
    void resetChat();
    QWORD getFriendChatID() const { return m_qwFriendChatID; }
    void setFriendChatID(QWORD userid) { m_qwFriendChatID = userid; }
  private:
    SceneUser* m_pUser = nullptr;

    TListChatCount m_listChatCount;
    TListChatItem m_listChatItem;
    TSetQWORD m_setSaveList;
    map<QWORD, DWORD> m_mapUpdateList;
    QWORD m_qwFriendChatID = 0;
};
