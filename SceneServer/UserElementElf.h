#pragma once

#include "xNoncopyable.h"
#include "xDefine.h"
#include "SceneNpc.h"
#include "RecordCmd.pb.h"

using namespace Cmd;

class UserElementElf : private xNoncopyable
{
public:
  UserElementElf(SceneUser* user);
  virtual ~UserElementElf();

public:
  bool load(const BlobElementElfData& data);
  bool save(BlobElementElfData* data);

  void timer(DWORD cur);
  void reset();

  SceneNpc* getCurElementNpc();
  DWORD getCurElementID() { return m_dwCurElementID; }
  QWORD getMasterCurLockID() { return m_qwMasterCurLockID; }
  QWORD getCurElementGUID() { SceneNpc* npc = getCurElementNpc(); return npc ? npc->getGUID() : 0; }
  bool enterScene(DWORD id);
  bool leaveScene();
  bool summon(DWORD id, DWORD last_seconds);

  void onUserEnterScene();
  void onUserLeaveScene();
  void onUserMove(const xPos& dest);
  void onUserGoTo(const xPos& dest);
  void onUserDie();
  void onUserAttack(xSceneEntryDynamic* enemy);
  void onProfesChange();

private:
  SceneUser* m_pUser = nullptr;
  DWORD m_dwCurElementID = 0;
  QWORD m_qwCurElementTempID = 0;
  DWORD m_dwClearTime = 0;
  QWORD m_qwMasterCurLockID = 0;
  DWORD m_dwTempEndBattleTime = 0;
};
