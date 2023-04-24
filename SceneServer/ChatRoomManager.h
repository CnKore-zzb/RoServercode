#ifndef __CHATROOMMANAGER_H__
#define __CHATROOMMANAGER_H__

#include "ChatRoom.h"
#include "xSingleton.h"

using TMapRoom = map<DWORD, ChatRoom*>;

class ChatRoomManager : public xSingleton<ChatRoomManager>
{
  friend class xSingleton<ChatRoomManager>;
  private:
    ChatRoomManager();
  public:
    virtual ~ChatRoomManager();

    void timer(DWORD curSec);
    ChatRoom* createRoom(SceneUser *pUser, const string &name, const string &pswd, DWORD maxnum);
    bool joinRoom(SceneUser *pUser, DWORD roomid, string pswd);
    bool exitRoom(SceneUser *pUser, DWORD roomid);
    bool kickMember(SceneUser* pUser, DWORD roomid, QWORD memberid);
    bool changeOwner(SceneUser* pUser, QWORD ownerid);

    bool broadcastRoomCmd(DWORD roomid, const void *cmd, DWORD len);
  protected:
    void checkValidRoom();

  public:
    ChatRoom* getRoom(DWORD roomid);
  private:
    bool addRoom(ChatRoom *);
    bool delRoom(ChatRoom *);
  private:
    TMapRoom m_mapRoom;
};


#endif
