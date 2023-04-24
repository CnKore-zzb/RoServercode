/**
 * @file SceneGameTip.h
 * @brief
 * @author tianyiheng@xindong.com
 * @version v1
 * @data 2015-09-14
 */

#pragma once

#include "SceneTip.pb.h"
#include "SceneManual.pb.h"
#include "Portrait.h"
#include "TableStruct.h"
#include "ItemManager.h"
#include <map>
#include <vector>
#include <bitset>
#include <tuple>

using namespace std;

// tip flag
enum ETipFlag
{
  ETIPFLAG_UNSEND = 0,
  ETIPFLAG_SENDED = 1,
  ETIPFLAG_BROWSED = 2
};

// tip map             tip sys  source ids
using TMapRedTip = map<ERedSys, TVecQWORD>;

class SceneUser;
namespace Cmd
{
  class BlobTips;
};

class SceneGameTip
{
  public:
    SceneGameTip(SceneUser *pUser);
    ~SceneGameTip();

    bool save(Cmd::BlobTips *data);
    bool load(const Cmd::BlobTips &data);

    bool addRedTip(ERedSys eRedSys, QWORD qwSubTip = 0);
    bool removeRedTip(ERedSys eRedSys, QWORD tipid = 0);
    void sendRedTip(ERedSys eRedSys);
    void clearUpdate() { m_bitMask.reset(); }

    //bool isTiped(ERedSys eRedSys, DWORD tipID);
    //bool isTiped(ERedSys eRedSys);

    void onPortrait(DWORD dwID);
    void onRolePoint();
    void onSkillPoint();

    void timer(DWORD curTime);

    bool browseTip(ERedSys eRedSys, QWORD tipid = 0);
    bool handleRedTip(const GameTipCmd &cmd);

    void patch_1();
  private:
    void update(DWORD curTime);
    void initDefaultTip();
  private:
    SceneUser* m_pUser = nullptr;

    TMapRedTip m_mapRedTip;
    bitset<EREDSYS_MAX> m_bitMask;
};

