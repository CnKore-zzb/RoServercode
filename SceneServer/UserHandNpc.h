#pragma once

#include "xNoncopyable.h"
#include "xDefine.h"
#include "RecordCmd.pb.h"
#include "SceneMap.pb.h"
#include "SysmsgConfig.h"

using namespace Cmd;

using namespace std;
using std::string;

struct SHandNpcData
{
  DWORD dwBody = 0;
  DWORD dwHead = 0;
  DWORD dwHair = 0;
  DWORD dwHairColor = 0;
  QWORD qwGuid = 0;
  DWORD dwSpEffectID = 0;
  DWORD dwEye = 0;
  string strName;

  void toData(HandNpcData* pData);
  void copyFrom(const SHandNpcData& data)
  {
    dwBody = data.dwBody;
    dwHead = data.dwHead;
    dwHair = data.dwHair;
    dwHairColor = data.dwHairColor;
    qwGuid = data.qwGuid;
    dwSpEffectID = data.dwSpEffectID;
    strName = data.strName;
    dwEye = data.dwEye;
  }
};

class SceneUser;
class UserHandNpc
{
  public:
    UserHandNpc(SceneUser* user);
    ~UserHandNpc();

  public:
    void save(BlobHandNpc* pData);
    void load(const BlobHandNpc& data);

  public:
    void timer(DWORD dwCurTime);
    bool addHandNpc();
    void delHandNpc();
    bool haveHandNpc() const { return m_dwEndTime != 0; }
    bool addHandNpc(const SHandNpcData& data, DWORD time);

    void getHandNpcData(HandNpcData* pData);

    void onLeaveScene();
    void onUseSkill();
  private:
    void playDialog(DWORD id);
    void playEmoji(DWORD id);
    bool checkAdd();
    bool canTalk() const { return m_stHandData.strName == SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_AVA); }
  private:
    SceneUser* m_pUser = nullptr;
    SHandNpcData m_stHandData;

    DWORD m_dwEndTime = 0;
    DWORD m_dwNextPlayEmoji = 0;
    DWORD m_dwNextPlayDialog = 0;
};
