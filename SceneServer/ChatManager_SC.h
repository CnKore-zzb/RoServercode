/**
 * @file ChatManager_SC.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2016-09-18
 */

#pragma once

#include "xSingleton.h"
#include "ChatCmd.pb.h"
#include "SessionCmd.pb.h"

using namespace Cmd;
using std::map;
class SceneUser;
const DWORD BARRAGE_INDEX = 10000;

// msg cache
typedef map<DWORD, string> TMapChatCache;
typedef map<QWORD, TMapChatCache> TMapUserChatCache;

class ChatManager_SC : public xSingleton<ChatManager_SC>
{
  friend class xSingleton<ChatManager_SC>;
  private:
    ChatManager_SC();
  public:
    virtual ~ChatManager_SC();

    void onLeaveScene(SceneUser* pUser);

    bool doChatCmd(SceneUser* pUser, const BYTE* buf, WORD len);
    bool doSocialCmd(const BYTE* buf, WORD len);

    bool queryVoiceData(SceneUser* pUser, DWORD dwVoiceID) const;
    bool processChatCmd(SceneUser* pUser, const ChatCmd& cmd);
    // 处理释放技能时发送的消息
    bool skillChat(SceneUser* pUser, DWORD sysmsgid);
    bool setBarrageUser(SceneUser* pUser, const BarrageChatCmd& cmd);
    bool sendBarrageMsg(SceneUser* pUser, const BarrageMsgChatCmd& cmd);
    bool getVoiceID(SceneUser* pUser);

    EZoneStatus getZoneStatus(DWORD dwZoneID) const;
    DWORD getZoneMaxBaseLv(DWORD dwZoneID) const;
    void setZoneInfo(const QueryZoneStatusSessionCmd& cmd);
    void toData(QueryZoneStatusUserCmd& cmd, SceneUser* pUser);
    bool checkIgnoreGagTime(EGameChatChannel eType);

    void sendChatMsgToNine(SceneUser* pUser, const char* str, EGameChatChannel channel);
    void sendChatMsgToMe(SceneUser* pUser, const char* str, EGameChatChannel channel);
  private:
    void chatLog(QWORD sID, QWORD sAccID, DWORD sPlat, DWORD sZone, DWORD sLv, QWORD rID, QWORD rAccID, const string rname, const ChatRetCmd& cmd);
  private:
    TMapUserChatCache m_mapUserChatCache;
    QueryZoneStatusSessionCmd m_oZoneCmd;

  private:
    bool chatRetFill(SceneUser* pUser, ChatRetCmd& ret, const ChatCmd& cmd);
    /*
     * bSkillChat: 是否是释放技能时候发出的消息
     * needReturn: 结束是否发生错误,调用处是否需要执行下去
     * bPunish: 玩家是否被禁言
     * sysmsgid: 技能释放发出的消息的msgid
     */
    bool roundChat(SceneUser* pUser, ChatSocialCmd& ret, bool bSkillChat, bool& needReturn, bool bPunish = false, DWORD sysmsgid = 0);
};

