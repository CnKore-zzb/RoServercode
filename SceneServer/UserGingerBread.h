#pragma once
/************************************************************************/
/* 姜饼人                                                                     */
/************************************************************************/
#include "xNoncopyable.h"
#include "xDefine.h"
#include "RecordTrade.pb.h"
#include "SceneTrade.pb.h"
#include "SceneMap.pb.h"
#include "xPos.h"

using namespace Cmd;

class SceneUser;

struct GingerBread
{
  GingerBreadNpcData blobInfo;
  DWORD nextDialogTime = 0;
  //消息前1分钟 提示消息
  void sendMsgBeforeDel(QWORD charId, DWORD curSec);
  bool bSend = false;
};

typedef std::map<QWORD/*giveid*/, GingerBread> TMapId2Ginger;

class UserGingerBread
{
  public:
    UserGingerBread(SceneUser* user);
    ~UserGingerBread();
    
    void onEnterScene();
    void onLeaveScene();

  public:
    void syncFromSession(const Cmd::SyncGiveItemSceneTradeCmd& rev);
    void timer(DWORD dwCurTime);
    bool addOne(EGiveType type,  QWORD id, DWORD expiteTime);
    bool delOne(EGiveType type, QWORD id, bool accept, bool isExpire);
    void collectNineData(Cmd::MapUser* pData);
    void sendLotterLetter2Client(const Cmd::ItemData& itemData);
    void resetGingerPos(const xPos& pos);
  private:
    void playDialog(QWORD guid, DWORD id);
    void playEmoji(DWORD id);
    bool isExpire(DWORD expireTime);
    inline void setDialogId();
  private:
    SceneUser* m_pUser = nullptr;
    DWORD m_dwDialogId = 0;
    std::map<EGiveType, TMapId2Ginger> m_mapGingerBread;
};
