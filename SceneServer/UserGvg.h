#pragma once

#include "xNoncopyable.h"
#include "xDefine.h"
#include "RecordCmd.pb.h"
#include "xTime.h"

using namespace Cmd;
using std::map;

struct SUserGvgData
{
  map<EGvgDataType, DWORD> mapType2Data;

  DWORD dwExpireTime = 0;

  void clear() { mapType2Data.clear(); dwExpireTime = 0; }
};

class SceneUser;
class UserGvg : private xNoncopyable
{
  public:
    UserGvg(SceneUser* user);
    virtual ~UserGvg();

  public:
    bool load(const BlobGvgData& oData);
    bool save(BlobGvgData* pData);

    void timer(DWORD cur);

    void onKillMonster();
    void onReliveUser();
    void onExpelOther(); //驱逐敌方
    void onDamMetal(DWORD per); // 伤害华丽金属, 万分比
    void onKillUser();
    void onKillMetal();

    void syncDataToMe();
    void onEnterGvg();
  private:
    bool checkInGvgFire();
    void givereward(DWORD itemid, DWORD count);
    void addCount(EGvgDataType eType, DWORD count);
    void onGvgGetHonor(DWORD honor);

    void updateToClient(EGvgDataType eType) const;
    void formatData(GvgData* pData, EGvgDataType eType, DWORD value) const;
  private:
    SceneUser* m_pUser = nullptr;
    SUserGvgData m_stGvgData;
    xTimer m_oTenSecTimer;
};
