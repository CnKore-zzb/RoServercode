#include "SceneDojo.h"
#include "SceneUser.h"
#include "DojoConfig.h"
#include "StatisticsDefine.h"
#include "GuildConfig.h"

// dojo
SceneDojo::SceneDojo(SceneUser* pUser) : m_pUser(pUser)
{
}

SceneDojo::~SceneDojo()
{
}

bool SceneDojo::load(const Cmd::BlobDojo &oBlob)
{
  for (int i = 0; i < oBlob.completedid_size(); ++i)
  {
    m_setCompleted.insert(oBlob.completedid(i));
  }  

  return true;
}

bool SceneDojo::save(Cmd::BlobDojo *data)
{
  for (auto it = m_setCompleted.begin(); it != m_setCompleted.end(); ++it)
  {
    data->add_completedid(*it);
  }

  XDBG << "[道场-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小 :" << data->ByteSize() << XEND;
  return true;
}

void SceneDojo::getPrivateInfo(Cmd::DojoPrivateInfoCmd& rev)
{
  for (auto it = m_setCompleted.begin(); it != m_setCompleted.end(); ++it)
  {
    const SDojoItemCfg* pCfg = DojoConfig::getMe().getDojoItemCfg(*it);
    if (!pCfg) continue;
    if (pCfg->dwGroupId != rev.groupid()) continue;
    rev.add_completed_id(*it);
  }
  
  PROTOBUF(rev, send, len);
  m_pUser->sendCmdToMe(send, len);
}

bool SceneDojo::isOpen(DWORD dwDojoGroup)
{
  const SGuildCFG* pCfg = GuildConfig::getMe().getGuildCFG(m_pUser->getGuild().lv());
  if (!pCfg)
    return false;
  return pCfg->isDojoGroupOpen(dwDojoGroup);
}

bool SceneDojo::isPassed(DWORD dwDojoId)
{
  return m_setCompleted.find(dwDojoId) != m_setCompleted.end();
}

bool SceneDojo::isGroupPassed(DWORD dwID)
{
  TSetDWORD setIDs;
  DojoConfig::getMe().collectDojoID(dwID, setIDs);
  if (setIDs.empty() == true)
    return false;
  for (auto &s : setIDs)
  {
    if (isPassed(s) == false)
      return false;
  }
  return true;
}

bool SceneDojo::passDojo(DWORD dwDojoId)
{
  XLOG << "[道场-通关] 成功通关道场:accid:" << m_pUser->accid<<m_pUser->id << "dojoid:" << dwDojoId << XEND;
  m_setCompleted.insert(dwDojoId);  
  m_pUser->getAchieve().onPassDojo(false);
  return true;
}
