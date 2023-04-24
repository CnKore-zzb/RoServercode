#pragma once

#include "xDefine.h"
#include "SceneUser2.pb.h"
#include "RecordCmd.pb.h"
#include "TutorConfig.h"
#include "MiscConfig.h"

using namespace Cmd;
using std::map;

class SceneUser;

struct STutorTaskItem
{
  DWORD dwID = 0;
  DWORD dwProgress = 0;
  bool bReward = false;

  bool fromData(const TutorTaskItem& rData);
  bool toData(TutorTaskItem* pData);
  void reset()
  {
    dwProgress = 0;
    bReward = false;
  }
};
typedef map<DWORD, STutorTaskItem> TMapTutorTaskItem;

struct STutorRewardItem
{
  QWORD qwCharID = 0;
  string strName;
  vector<DWORD> vecReward;
  vector<pair<DWORD, DWORD>> vecTaskID2Time;

  bool fromData(const TutorReward& data);
  bool toData(TutorReward* data);
  void addReward(const TutorReward& data);
};

class TutorTask
{
public:
  TutorTask(SceneUser* pUser);
  ~TutorTask();

  bool load(const BlobTutorTask& rData);
  bool save(BlobTutorTask* pData);

  void timer(DWORD cur);
  DWORD getProficiency();
  DWORD isProficiencyMax() { return getProficiency() >= MiscConfig::getMe().getTutorCFG().dwMaxProfrociency; }
  void sendTaskData();
  void sendGrowRewardData();
  bool canBeTutor();
  bool graduation();

  void addTutorReward(const TutorReward& reward);
  void sendTutorRewardMail();
  bool getTutorReward(QWORD studentid, DWORD taskid);
  void queryStudentTask(TutorTaskQueryCmd& cmd);
  void sendRedPointToTutor();
  void addRedPointTip(QWORD tip);
  bool getGrowReward(DWORD level = 0);
  void sendGrowRewardMail();

  void onLevelUp();
  void onQuestSubmit(DWORD questid);
  void onRepairSeal();
  void onItemUsed(DWORD itemid);
  void onItemBeUsed(DWORD itemid);
  void onPassTower(DWORD curlayer, DWORD premaxlayer);
  void onGuildRaidFinish();
  void onLaboratoryFinish();
  void onEnterScene();
  void onLeaveScene();
  void onUpdateSocial();
  void onTutorChanged(QWORD oldtutor, QWORD newtutor);

private:
  void addProficiency(DWORD value);
  void clearProficiency();
  void updateProgress(ETutorTaskType type, DWORD progress = 1, QWORD value = 0);
  void resetProgress();
  DWORD getSnapshotLevel();
  bool isGrowRewardGot(DWORD lv);
  void setGrowRewardGot(DWORD lv, bool ntf = true);
  bool isTutorGrowRewardGot(DWORD lv);
  void setTutorGrowRewardGot(DWORD lv);
  void saveTaskToRedis();
  void refreshTutorRewardRedPoint();
  void refreshGrowRewardRedPoint();

  SceneUser* m_pUser;
  xTimer m_oOneMinTimer;
  xTimer m_oFiveMinTimer;
  DWORD m_dwProficiency = 0;
  map<ETutorTaskType, TMapTutorTaskItem> m_mapType2Items;
  vector<QWORD> m_vecGrowReward;
  vector<QWORD> m_vecTutorGrowReward;
  map<QWORD, STutorRewardItem> m_mapCharID2Reward;
  bool m_bRedPoint = false;
  bool m_bProgressUpdate = false;
  DWORD m_dwGrowRewardLv = 0;
};
