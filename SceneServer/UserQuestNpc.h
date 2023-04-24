#pragma once

#include "xDefine.h"
#include "SceneDefine.h"
#include <list>

class SceneUser;
class SceneNpc;

struct QuestNpcSave
{
  QWORD m_qwTempID = 0;

  DWORD m_dwMapID = 0;
  DWORD m_dwQuestID = 0;
  DWORD m_dwGroupID = 0;
  DWORD m_dwHp = 0;

  NpcDefine m_oDefine;

  bool bDelete = false;
  bool bAcc = false;

  QuestNpcSave() {}
};

namespace Cmd
{
  class BlobQuestNpc;
};

class UserQuestNpc
{
  public:
    UserQuestNpc(SceneUser *u);
    ~UserQuestNpc() {}

    void onUserEnterScene();
    void onUserLeaveScene();
    void onNpcDie(SceneNpc *npc);
    bool addNpc(SceneNpc *npc, DWORD dwQuestID);
    bool addNpc(const NpcDefine& rDefine, DWORD dwMapID, DWORD dwQuestID);
    void save(Cmd::BlobQuestNpc *acc_data, Cmd::BlobQuestNpc *data);
    void load(const BlobQuestNpc &acc_data, const Cmd::BlobQuestNpc &data);

    void delNpc(DWORD id, DWORD dwQuestID);
    void killNpc(DWORD id, DWORD uniqueID);
    void delNpcQuest(DWORD dwQuestID, DWORD dwGroupID = 0);

    void getCurMapNpc(std::set<SceneNpc*> &npcset);

    void setQuestNpcInfo(DWORD dwQuestID, DWORD dwNpcID, DWORD dwMapID);
    void setQuestStatus(DWORD dwQuestID, bool bAcc);
  public:
    std::list<QuestNpcSave> m_list;
  private:
    SceneUser *m_pUser;
};
