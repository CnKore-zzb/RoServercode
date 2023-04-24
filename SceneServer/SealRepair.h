#pragma once

#include "xDefine.h"
#include "xPos.h"
#include "SealConfig.h"

using namespace Cmd;
using std::string;
using std::vector;
using std::set;

// seal data
struct SSealItem
{
  QWORD qwSealID = 0;
  DWORD dwRefreshTime = 0;

  const SealCFG* pCFG = nullptr;

  xPos oPos;
  bool bOwnSeal = false;
  bool bSealing = false;

  SSealItem() {}

  bool fromData(const SealItem& rItem);
  bool toData(SealItem* pItem) const;
  bool toClient(SealItem* pItem) const;
};
typedef vector<SSealItem> TVecSealItem;

struct SSealData
{
  DWORD dwMapID = 0;

  TVecSealItem vecItem;

  SSealData() {}

  bool fromData(const SealData& rData);
  bool toData(SealData* pData) const;
  bool toClient(SealData* pData) const;
  bool haveSeal() const;

  const SSealItem* getItem(QWORD qwID) const;
};
typedef vector<SSealData> TVecSealData;

class SceneUser;
class SceneTeam;
enum ESealStatus
{
  ESealStatus_Min = 0,
  ESealStatus_Process = 1,
  ESealStatus_RepairOk = 2,
  ESealStatus_Finish = 3,
  ESealStatus_Exit = 4,
};
struct SRepairSeal
{
  DWORD dwMapID = 0;
  DWORD dwSealStopTime = 0;
  QWORD qwActID = 0;

  DWORD dwCurValue = 0;
  DWORD dwNextProcessTime = 0;
  DWORD dwFinishTime = 0;
  int intCurSpeed = 0;

  ESealStatus eStatus = ESealStatus_Process;

  SSealItem m_stCurItem;

  set<QWORD> m_setActUserID;
  set<QWORD> m_setCalledMonster;

  //SceneTeam* pOwnTeam = nullptr;
  SceneUser* pOwnUser = nullptr;

  bool checkRepairOk();
  bool checkOk(DWORD curTime);
  bool checkFailure(DWORD curTime);
  bool checkExitAction(DWORD exitTime);
  void callMonster();

  void process(DWORD curTime);
  void finishSeal();
  void failSeal();
  void preFinishSeal();

  void findActUser();
  void clear();

  SRepairSeal() {}
};
typedef vector<SRepairSeal> TVecRepairSeal;


