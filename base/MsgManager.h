/**
 * @file SysMsgParam.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v2
 * @date 2016-05-04
 */

#pragma once

#include "xDefine.h"
#include "SceneUser2.pb.h"
#include "GateSuper.pb.h"
#include "ChatCmd.pb.h"
#include "xSingleton.h"

using std::vector;
using std::string;
using std::ostringstream;
using std::set;
using namespace Cmd;

struct RoleData;


// msg params
class MsgParams
{
  friend class MsgManager;
  public:
    MsgParams();
    MsgParams(DWORD value);
    MsgParams(const string& str);
    MsgParams(DWORD value1, DWORD value2);
    MsgParams(const string& str, DWORD value);
    MsgParams(const string& str, float value);
    MsgParams(const RoleData* pData, float value);
    MsgParams(const string& str1, const string& str2);
    MsgParams(const string& str1, const string& str2, const string& str3);
    MsgParams(const string& str, DWORD value, DWORD value2);
    MsgParams(const TVecString& subparams);

    ~MsgParams();

    bool addNumber(DWORD value);
    bool addFloat(float value);
    bool addString(const string& str);

    bool addSubParam();
    bool addSubNumber(DWORD value);
    bool addSubFloat(float value);
    bool addSubString(const string& str);

    bool addLangParam();
    bool addLangParam(const MsgLangParam& langparam);

    void toData(SysMsg& rMsg) const;
    void clear() { m_vecParams.clear(); }
    void toNpcChatNtf(NpcChatNtf& ntf) const;

    static string fmtString(std::string fmt, MsgParams params);
    const vector<MsgParam>& getMsgParams() const { return m_vecParams; }
  private:
    vector<MsgParam> m_vecParams;
};

// broad info
struct SBroadcastInfo
{
  DWORD dwID = 0;

  DWORD dwMsgID = 0;
  DWORD dwStartTime = 0;
  DWORD dwInterval = 0;
  DWORD dwCount = 0;
  DWORD dwMaxCount = 0;

  MsgParams oParams;

  SBroadcastInfo() {}
};
typedef vector<SBroadcastInfo> TVecBroadcastInfo;

// msg manager
class MsgManager
{
  private:
    MsgManager();
    ~MsgManager();

  public:
    static void sendMsg(QWORD qwCharID, DWORD dwMsgID, const MsgParams& params = MsgParams(), EMessageType eType = EMESSAGETYPE_FRAME, EMessageActOpt eAct = EMESSAGEACT_ADD, DWORD dwDelay = 0);
    //
    static void sendMsgMust(QWORD qwCharID, DWORD dwMsgID, const MsgParams& params = MsgParams(), EMessageType eType = EMESSAGETYPE_FRAME, EMessageActOpt eAct = EMESSAGEACT_ADD, DWORD dwDelay = 0);
    static void sendDebugMsg(QWORD qwCharID, const string& str);
    static void sendRoomMsg(DWORD dwRoomID, DWORD dwMsgID, const MsgParams& params = MsgParams(), EMessageType eType = EMESSAGETYPE_FRAME);
    static void sendMapMsg(DWORD dwMapID, DWORD dwMsgID, const MsgParams& params = MsgParams(), EMessageType eType = EMESSAGETYPE_FRAME);
    static void sendWorldMsg(DWORD dwMsgID, const MsgParams& params = MsgParams(), EMessageType eType = EMESSAGETYPE_FRAME);

    static void sendMapCmd(DWORD dwMapID, const void* cmd, unsigned short len);
    static void sendWorldCmd(const void* cmd, unsigned short len);

    static void alter_msg(const string& strTitle, const string& strMsg, const EPushMsg& event);

    /*action:0:del 1:modify 2:add*/
    static DWORD addBroadInfo(DWORD action, DWORD dwId, const string& starttime, const string& str, DWORD dwMsgID, DWORD dwInterval, DWORD dwCount, const MsgParam& langparam);
    static bool delBroad(DWORD dwId);
    static void timer(DWORD dwCurTime);

  private:
    static void processBroadcast(DWORD dwCurTime);
  private:
    static TVecBroadcastInfo m_vecBroadcastInfo;
};

class RealtimeVoiceManager : public xSingleton<RealtimeVoiceManager>
{
  friend class xSingleton<RealtimeVoiceManager>;
private:
  RealtimeVoiceManager() {}
public:
  virtual ~RealtimeVoiceManager() {}

  const string& getID();

private:
  string getRandID();

  set<string> m_setUsedID;
};
