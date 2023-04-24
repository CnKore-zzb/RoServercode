#pragma once

#include "xDefine.h"
#include "xNoncopyable.h"
#include "GuildSCmd.pb.h"
#include "RecordCmd.pb.h"
#include "GuildConfig.h"

using namespace Cmd;
class SceneUser;
class SceneNpc;

struct SGuildChallengeData
{
  DWORD dwID = 0;
  DWORD dwProgress = 0;

  bool toData(GuildChallengeItem* data)
  {
    if (data == nullptr)
      return false;
    data->set_id(dwID);
    data->set_progress(dwProgress);
    return true;
  }
  bool fromData(const GuildChallengeItem& data)
  {
    dwID = data.id();
    dwProgress = data.progress();
    return true;
  }
};

class GuildChallengeMgr : private xNoncopyable
{
public:
  GuildChallengeMgr(SceneUser* user);
  virtual ~GuildChallengeMgr();

  bool load(const BlobGuildChallenge& data);
  bool save(BlobGuildChallenge* data);

  void sendChallengeToGuild(TVecDWORD ids);
  void resetProgress(bool force = false);
  void updateProgress(EGuildChallenge type, DWORD progress = 1, QWORD value = 0);
  
  void onLogin();
  void onGuildRaidFinish();
  void onEnterGVG();
  void onPassTower(DWORD curlayer);
  void onRepairSeal();
  void onQuestSubmit(DWORD questid);
  void onKillNpc(SceneNpc* pNpc);

private:
  SceneUser* m_pUser = nullptr;

  map<DWORD, SGuildChallengeData> m_mapChallenge;
};
