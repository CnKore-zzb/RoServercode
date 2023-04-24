#include "PatchManager.h"
#include "xLuaTable.h"
#include "SceneUser.h"
#include "SceneUserManager.h"

PatchManager::PatchManager()
{

}

PatchManager::~PatchManager()
{

}

bool PatchManager::loadPatchList()
{
  /*if (xLuaTable::getMe().open("Patch/quest_charid_list.lua") == false)
  {
    XERR << "[任务管理-名单加载] 加载变身叶子名单失败" << XEND;
    return false;
  }

  m_setQuestCharIDs.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("QuestCharID", table);
  for (auto m = table.begin(); m != table.end(); ++m)
    m_setQuestCharIDs.insert(m->second.getInt());

  XLOG << "[任务管理-名单加载] 成功加载" << m_setQuestCharIDs.size() << "个名单" << XEND;
  return true;*/

  if (!thisServer->addDataBase(REGION_DB, false))
  {
    XERR << "[补丁管理-名单加载],DBPool,初始化失败," << REGION_DB << XEND;
    return false;
  }

  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "quest_patch");
  if (pField == nullptr)
  {
    XERR << "[补丁管理-名单加载] 加载变身叶子名单失败, 未发现 quest_patch 数据库表" << XEND;
    return false;
  }

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, nullptr);
  if (ret == QWORD_MAX)
  {
    XERR << "[补丁管理-名单加载] 加载变身叶子名单失败, 查询失败 ret :" << ret << XEND;
    return false;
  }
  m_setQuestCharIDs.clear();
  for (DWORD d = 0; d < set.size(); ++d)
    m_setQuestCharIDs.insert(set[d].get<QWORD>("charid"));

  thisServer->delDataBase();
  XLOG << "[补丁管理-名单加载] 成功加载" << m_setQuestCharIDs.size() << "个名单" << XEND;
  return true;
}

// patch
bool PatchManager::canPatchAccept(SceneUser* pUser, DWORD id)
{
  if (pUser == nullptr)
    return false;
  if (isPatchChar(pUser->id) == false)
    return true;
  if (id != 390040001 && id != 390050001)
    return true;
  return pUser->getQuest().isSubmit(600010001);
}

bool PatchManager::submitPatch(SceneUser* pUser, DWORD id)
{
  if (pUser == nullptr)
    return false;
  if (isPatchChar(pUser->id) == false)
    return true;
  if (id != 600010001)
    return true;

  DelPatchCharRecordCmd cmd;
  cmd.set_charid(pUser->id);
  cmd.set_type(EPATCHTYPE_QUEST);
  PROTOBUF(cmd, send, len);
  bool bResult = thisServer->sendCmdToRecord(send, len);
  if (bResult)
    XLOG << "[任务-补丁(变身叶子)]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "提交了补丁任务,成功发送至RecordServer处理" << id << XEND;
  else
    XERR << "[任务-补丁(变身叶子)]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "提交了补丁任务,发送至RecordServer处理失败" << id << XEND;
  return bResult;
}

