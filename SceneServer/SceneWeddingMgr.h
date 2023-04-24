#pragma once

#include "xEntryManager.h"
#include "xSingleton.h"
#include "xCommand.h"
#include "WeddingSCmd.pb.h"

class SceneWeddingMgr : public xSingleton<SceneWeddingMgr>
{
  public:
    void startWedding(Cmd::StartWeddingSCmd& cmd);
    void stopWedding(Cmd::StopWeddingSCmd& cmd);
    
    bool isCurWedding(QWORD qwId) { return m_oCurWedding.id()==0? false: m_oCurWedding.id() == qwId; }

    const WeddingInfo& getWeddingInfo() const { return m_oCurWedding; }
    bool isCurWeddingUser(QWORD userid) { return m_oCurWedding.charid1() == userid || m_oCurWedding.charid2() == userid; }

    void setCurWeddingSceneID(QWORD sceneid) { m_qwCurWeddingScenID = sceneid; }
    void addSceneEvent(QWORD eventid) { m_setSceneEventIDs.insert(eventid); }

    void updateManual(QWORD id, const WeddingManualInfo& info);
    bool hasServiceID(DWORD id) const;

    void onMarrySuccess(QWORD id);
  private:
    void doServiceEffect(DWORD id);
    void doMarrySuccessEffect(DWORD id);
  private:
    WeddingInfo m_oCurWedding;
    QWORD m_qwCurWeddingScenID = 0;
    TSetQWORD m_setSceneEventIDs;
};
