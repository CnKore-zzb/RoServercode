#ifndef __USER_MESSAGE_H__
#define __USER_MESSAGE_H__

/**
 * @file UserMessage.h
 * @brief
 * @author tianyiheng, tianyiheng@xindong.com
 * @date 2015-10-10
 */

#include <list>
#include "SceneUser2.pb.h"
#include "xDefine.h"

namespace Cmd
{
  class BlobChatMsg;
};

using std::string;
using std::pair;
using std::list;
using namespace Cmd;

using TPresetMsg = pair<DWORD, std::string>;
using TListPreMsg = list<TPresetMsg>;

class SceneUser;
class UserMessage
{
  public:
    UserMessage(SceneUser *pUser);
    ~UserMessage(){};

    bool load(const BlobChatMsg& oBlob);
    bool save(BlobChatMsg* pBlob);
    bool toClient();
    bool addPreMsg(const string& sPreMsg);
    bool resetMsgs(const Cmd::PresetMsgCmd & cmd);
  private:
    TListPreMsg m_listPreMsg;
    SceneUser *m_pUser = nullptr;
};

#endif
