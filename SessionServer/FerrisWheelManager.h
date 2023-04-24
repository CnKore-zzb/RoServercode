/**
 * @file FerrisWheelManager.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2016-09-08
 */

#pragma once

#include "xSingleton.h"
#include "CarrierCmd.pb.h"
#include "SessionCmd.pb.h"
#include "DateLandConfig.h"

using std::vector;
using std::map;
using namespace Cmd;

struct SFerrisPartner
{
  QWORD qwCharID = 0;
  DWORD dwTime = 0;
};
typedef vector<SFerrisPartner> TVecFerrisPartner;
typedef map<QWORD, TVecFerrisPartner> TMapFerrisPartner;
typedef map<DWORD, TMapFerrisPartner> TMapID2FerrisPartner;

struct SFerrisReady
{
  QWORD qwMaster = 0;
  QWORD qwTarget = 0;

  DWORD dwTime = 0;
};
typedef vector<SFerrisReady> TVecFerrisReady;
typedef map<DWORD, TVecFerrisReady> TMapID2FerrisReady;

class SessionUser;
class FerrisWheelManager : public xSingleton<FerrisWheelManager>
{
  friend class xSingleton<FerrisWheelManager>;
  private:
    FerrisWheelManager();
  public:
    virtual ~FerrisWheelManager();

    bool invite(DWORD dwID, SessionUser* pUser, QWORD qwTargetID);
    bool processInvite(DWORD dwID, SessionUser* pUser, QWORD qwTargetID, EFerrisAction eAction);
    bool ready(const EnterFerrisReadySessionCmd& cmd);

    void timer(DWORD curTime);
  private:
    SFerrisPartner* getPartner(DWORD dwID, QWORD qwMaster, QWORD qwPartner);
    bool removeInvite(DWORD dwID, QWORD qwCharID);
    bool canInvite(DWORD dwID, SessionUser* pUser);
  private:
    TMapID2FerrisPartner m_mapFerrisPartner;
    TMapID2FerrisReady m_mapFerrisReady;
};

