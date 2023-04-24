#pragma once

#include <set>
#include "xNoncopyable.h"
#include "xDefine.h"
#include "SceneUser2.pb.h"
#include "DeathTransferMapConfig.h"

#define DEATH_MAP1_ID 71
#define DEATH_MAP2_ID 72
#define WING_OF_FLY_ID 5024

class SceneUser;
class SceneNpc;

namespace Cmd
{
  class BlobTransfer;
};
using namespace Cmd;

class Transfer : private xNoncopyable
{
  public:
    Transfer(SceneUser *u);
    virtual ~Transfer();

    void save(Cmd::BlobTransfer *data);
    void load(const Cmd::BlobTransfer &data);

    void goTransfer(const Cmd::UseDeathTransferCmd& cmd);
    void sendList();

    bool hasActivated(DWORD npcid);
    bool activateTransfer(DWORD npcid, bool bNotify = true);
    bool canUseWingOfFly(DWORD mapid);
  private:
    bool canTransfer(const SDeathTransferConfig* from, const SDeathTransferConfig* to);

    SceneUser *m_pUser = nullptr;

    TSetDWORD m_setActiveTransferIDs;
    bool m_bMap1AllActivated;
    bool m_bMap2AllActivated;
};
