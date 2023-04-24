#pragma once

#include <set>
#include "xNoncopyable.h"
#include "xDefine.h"
#include "SceneUser2.pb.h"

class SceneUser;
class SceneNpc;

namespace Cmd
{
  class BlobFreyja;
};
using namespace Cmd;

class Freyja : private xNoncopyable
{
  public:
    Freyja(SceneUser *u);
    virtual ~Freyja();

    void save(Cmd::BlobFreyja *data);
    void load(const Cmd::BlobFreyja &data);

    void goToGear(const Cmd::GoToGearUserCmd& cmd);
    void sendList();

    bool isVisible(DWORD mapid);
    bool addFreyja(DWORD mapid, bool bNotify = true);
  private:
    SceneUser *m_pUser = nullptr;

    TVecDWORD m_vecActiveMapIDs;
};
