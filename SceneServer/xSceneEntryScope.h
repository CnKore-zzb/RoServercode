#pragma once

#include "xNoncopyable.h"
#include <unordered_map>
#include "xDefine.h"
#include "xSceneEntry.h"
#include "SceneMap.pb.h"

class SceneUser;
class xSceneEntryScope;
typedef std::unordered_set<SceneUser*> SceneUserSet;

class xScope : private xNoncopyable
{
  public:
    explicit xScope(SceneUser *user);
    ~xScope();

  public:
    // 添加到等待列表，会按规则过滤一遍
    void addToWaitList(SceneUser *pUser);
    // 直接添加到队列，不检查条件
    void addToOKList(SceneUser *pUser, bool isFriend);

    void remove(SceneUser *pUser, bool from = false);
    void refresh(xSceneEntryScope *pScope);
    inline DWORD getSizeExceptFriend()
    {
      DWORD dwFriendSize = m_setFriendList.size();
      DWORD dwSize = m_oOKList.size();

      if (dwSize > dwFriendSize)
      {
        return dwSize - dwFriendSize;
      }
      return 0;
    }
    inline DWORD getFriendSize()
    {
      return m_setFriendList.size();
    }
    bool has(SceneUser *pUser) { return m_oOKList.find(pUser) != m_oOKList.end(); }
    void destroy(xSceneEntryScope *pScope);
    void sendCmdToAll(const void* cmd, DWORD len);

    const SceneUserSet& getSceneUserSet() { return m_oOKList; }

    inline void setMaxScopeNum(DWORD dwNum, DWORD dwFriendNum)
    {
      if (dwNum)
      {
        MAX_SCOPE_USER_NUM = dwNum;
      }
      MAX_SCOPE_FRIEND_NUM = dwFriendNum;
    }
    void sendMeToScope();
    void sendMeToUser(SceneUser *pUser);
    void hide();

  private:
    void refreshNineUserToMe();
    bool addToOK(SceneUser *pUser, bool isFriend);
    bool isInOK(SceneUser *pUser);

  private:
    SceneUserSet m_oWaitList;
    SceneUserSet m_oOKList;

    SceneUserSet m_oAddList;

    SceneUser *m_pUser = nullptr;

    std::set<QWORD> m_setDeleteList;

    std::set<QWORD> m_setFriendList;

    bool m_blFull = false;

    DWORD MAX_SCOPE_USER_NUM = 50;
    DWORD MAX_SCOPE_FRIEND_NUM = 0;
};

class xSceneEntryScope : private xNoncopyable
{
  public:
    xSceneEntryScope();
    virtual ~xSceneEntryScope();

  public:
    static void sendCmdToSceneUsers(SceneUserSet &set, const void* data, unsigned short len);

  public:
    bool add(SceneUser *fromUser, SceneUser *toUser);
    bool remove(SceneUser *fromUser, SceneUser *toUser);
    xScope* get(QWORD id);
    xScope* get(SceneUser *pUser);
    bool inScope(SceneUser *fromUser, SceneUser *toUser);
    void destroy(SceneUser *pUser);

    bool getSceneUserSet(SceneUser *pUser, SceneUserSet& set);
    void sendCmdToScope(SceneUser *pUser, const void *cmd, DWORD len);
    void sendUserToScope(SceneUser *pUser);
    void sendUserToUser(SceneUser *pFromUser, SceneUser *pToUser);
    void hideMeToScope(SceneUser *pUser);

  private:
    // 视野
    std::unordered_map<QWORD, xScope*> m_oScopeList;
};
