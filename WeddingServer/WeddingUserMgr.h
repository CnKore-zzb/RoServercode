#include <map>
#include "xSingleton.h"
#include "SocialCmd.pb.h"
#include "MsgManager.h"

struct WeddingUser
{
  //Ô¤¶¨Ëø
  bool lockReserve();
  //Ô¤¶¨½âËø
  bool unlockReserve();
  void ntfBriefWeddingInfo2Client();

  QWORD m_qwCharId = 0;
  string m_strName;
  DWORD m_dwZoneId = 0;
  
  DWORD m_dwReserveLock = 0;

  bool sendCmdToMe(void* buf, WORD len);
  bool sendCmdToScene(void* buf, WORD len);
  void sendMsg(DWORD dwMsgID, MsgParams oParams = MsgParams(), EMessageType eType = EMESSAGETYPE_FRAME);
};

class WeddingUserMgr :public xSingleton<WeddingUserMgr>
{
public:
  void onUserOnline(const Cmd::SocialUser& rUser);
  void onUserOffline(const Cmd::SocialUser& rUser);
  bool syncWeddingInfo2Scene(QWORD qwCharId);
  bool syncWeddingInfo2Scene(QWORD qwCharId, DWORD dwZoneid);
  WeddingUser* getWeddingUser(QWORD qwCharId);
  bool sendCmdToUser(QWORD qwCharId, void* buf, WORD len);

private:
  
  std::map<QWORD, WeddingUser> m_mapUser;
};
