#ifndef __CHAT_ROOM_H__
#define __CHAT_ROOM_H__

/**
 * @file ChatRoom.h
 * @brief
 * @author tianyiheng, tianyiheng@xindong.com
 * @date 2015-10-13
 */

// include

// project
#include "xDefine.h"
#include "SceneChatRoom.pb.h"
#include "xCommand.h"
#include "xPool.h"
using namespace Cmd;


// std
/*
//#include <vector>
#include <string>
*/
#include <map>
#include <list>

#include <algorithm>
#include <functional>
using namespace std;

using std::map;
using std::list;

// class define

//using TVecQWORD = vector<QWORD>;

//#define _TYH_DEBUG

class SceneUser;

class RoomMember : public xObjectPool<RoomMember>
{
  public:
    RoomMember(QWORD userid, EChatRoomJob eRoomJob);
    bool init(SceneUser *pUser);

    bool toData(Cmd::ChatRoomMember& rMember);

    void setJob(Cmd::EChatRoomJob eRoomJob) { /*m_eRoomJob = eRoomJob;*/m_oData.set_job(eRoomJob); }
    EChatRoomJob getJob() const { return m_oData.job(); }
    QWORD getUserID() const { return m_oData.id(); }

    SceneUser* getUser();
  private:
    ChatRoomMember m_oData;
};


// chat room
using TMemberList  = list<RoomMember*>;
using TMapKick = map<QWORD, DWORD>;

// define
//#define TIME_LIMIT_KICK_SEC   300
//#define MAX_ROOM_MEMBER_NUM   10
//#define MIN_ROOM_MEMBER_NUM   2

struct SRoomData
{
  DWORD dwRoomID = 0;
  string sName;
  string sPswd;
  QWORD qwOwnerID = 0;
  EChatRoomType eRoomType;
  DWORD dwMaxNum = 0;
};

class ChatRoom : public xObjectPool<ChatRoom>
{
  public:
    ChatRoom(DWORD roomid, QWORD ownerid,const string &name, const string &pswd, DWORD maxnum);
    bool init(DWORD roomid, QWORD ownerid,const string &name, const string &pswd, DWORD maxnum);
    void timer(DWORD curSec);
    bool updateMember(DWORD curTimeSec);
    void updateKick(DWORD curTimeSec);
    void updateNineSync(DWORD curTimeSec);
    bool toClient(Cmd::ChatRoomData &rRoomData);
    bool toSummary(Cmd::ChatRoomSummary &rSummary);

    bool addMember(RoomMember* memberPtr, bool bUpdate = true);
    bool removeMember(QWORD userid);
    RoomMember* getMember(QWORD userid);
    bool isMember(QWORD userid);
    DWORD curMemberNum() { return m_lsMember.size(); };
    const TMemberList& getMemberList() const { return m_lsMember; }

    const EChatRoomType& getRoomType() const { return m_stBaseData.eRoomType; }
    const string& getPswd() const { return m_stBaseData.sPswd; }
    bool setOwner(QWORD ownerid);
    DWORD getRoomID() const { return m_stBaseData.dwRoomID; }
    const string& getRoomName() const { return m_stBaseData.sName; }
    bool addKickMember(QWORD userid);
    bool isInKick(QWORD userid);
    QWORD getOwnerID() { return m_stBaseData.qwOwnerID; }
    RoomMember* getFirstMember();
    RoomMember* getOwner();
    bool sendCmdToMember(const void* cmd, DWORD len);
    template<class T>
      void foreach(T func)
      {
        for_each(m_lsMember.begin(), m_lsMember.end(),[func](const std::list<RoomMember*>::value_type r){func(r);});
      }
    void setNineSync(bool bSync);
  private:
    SRoomData m_stBaseData;
    TMemberList m_lsMember;

    TVecQWORD m_vecUpdate;
    TMapKick m_mapKick;

    bool m_bNineSync;
};




#endif
