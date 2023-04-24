#pragma once

#include "xNoncopyable.h"
#include "xDefine.h"
#include "RecordCmd.pb.h"
#include "MiscConfig.h"

using namespace Cmd;
using std::map;

class SceneUser;

class UserCamera
{
  public:
    UserCamera(SceneUser* user);
    ~UserCamera();

  public:
    void load(const BlobCamera& data);
    void save(BlobCamera* pData);
  public:
    void onCamera();
    void clear() { m_mapMons2Num.clear(); }
  private:
    bool summon(const SCameraMonster& rCFG);
  private:
    SceneUser* m_pUser = nullptr;
    map<DWORD, DWORD> m_mapMons2Num;
    DWORD m_dwNextSummonTime = 0;
};
