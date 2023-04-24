#pragma once

#include "xNoncopyable.h"
#include "xDefine.h"
#include "RecordCmd.pb.h"
#include "SceneMap.pb.h"
#include <unordered_map>
#include <set>
#include "xPos.h"

class SceneUser;
class SceneNpc;

class UserItemMusic : private xNoncopyable
{
  public:
    UserItemMusic(SceneUser *u);
    ~UserItemMusic();
  public:
    bool hasMusicItem() { return !m_musicUri.empty(); };
    bool startMusicItem(std::string uri, DWORD dwExpireTime, DWORD dwNpcId, DWORD dwRange, DWORD dwNum, DWORD itemId);
    bool stopMusicItem();
    void sendMusicToMe(bool bOnlyHander, bool bAdd);
    void checkMusicItem(DWORD dwCurTime);
    void leaveScene();
    bool checkHeadEffect(DWORD itemId, DWORD actionId, EUserDataType dataType);
    void onActionChange(DWORD actionId);

  private:
    bool checkCanUse();
    EUserDataType convertEquipType2DataType(EEquipType equipType);

  private:
    SceneUser *m_pUser = NULL;
    std::string m_musicUri;
    DWORD m_dwMusicItemStartTime = 0;
    DWORD m_dwMusicItemEndTime = 0;
    TSetQWORD m_setNpcGuid ;
    xPos m_musicItemPos;
    DWORD m_dwMusicItemRange = 0;
    DWORD m_dwStartTime = 0;

    DWORD m_dwLastEffectItemId = 0;
    EUserDataType m_dataType = EUSERDATATYPE_MIN;
};
