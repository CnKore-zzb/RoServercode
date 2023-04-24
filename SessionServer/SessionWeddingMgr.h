#pragma once

#include "xEntryManager.h"
#include "xSingleton.h"
#include "xCommand.h"
#include "WeddingSCmd.pb.h"

class ServerTask;
class SessionWeddingMgr : public xSingleton<SessionWeddingMgr>
{
  public:
    void startWedding(Cmd::StartWeddingSCmd& cmd);
    void stopWedding(Cmd::StopWeddingSCmd& cmd);

    void syncWeddingInfo2Scene(ServerTask* pTask);
  private:
    WeddingInfo m_oCurWedding;
};
