#include "Interlocution.h"
#include "SceneUser.h"
#include "TableManager.h"
#include "GuidManager.h"

// data
bool SInterData::fromData(const InterData& rData)
{
  dwGUID = rData.guid();
  dwInterID = rData.interid();

  eSource = rData.source();
  return true;
}

bool SInterData::toData(InterData* pData)
{
  if (pData == nullptr)
    return false;

  pData->set_guid(dwGUID);
  pData->set_interid(dwInterID);
  pData->set_source(eSource);
  return true;
}

Interlocution::Interlocution(SceneUser* pUser) : m_pUser(pUser)
{

}

Interlocution::~Interlocution()
{

}

bool Interlocution::load(const BlobInter& rData)
{
  m_vecDatas.clear();

  for (int i = 0; i < rData.list_size(); ++i)
  {
    SInterData stData;
    stData.fromData(rData.list(i));

    m_vecDatas.push_back(stData);
  }

  return true;
}

bool Interlocution::save(BlobInter* rData)
{
  rData->Clear();
  for (auto v = m_vecDatas.begin(); v != m_vecDatas.end(); ++v)
  {
    InterData* pData = rData->add_list();
    if (pData == nullptr)
    {
      XERR << "[Interlocution::save]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id =" << v->dwGUID << "create protobuf error" << XEND;
      continue;
    }

    v->toData(pData);
  }

  XDBG << "[问答-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小 :" << rData->ByteSize() << XEND;
  return true;
}

void Interlocution::addInterlocution(DWORD dwID, ESource eSource)
{
  SInterData stData;
  stData.dwGUID = GuidManager::getMe().getNextInterID();
  stData.dwInterID = dwID;
  stData.eSource = eSource;
  m_vecDatas.push_back(stData);

  sendInterlocution();

  XLOG << "[问答-增加问题]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << dwID << "source :" << eSource << XEND;
}

void Interlocution::sendInterlocution()
{
  if (m_vecDatas.empty() == true)
    return;

  const SInterData& rData = *m_vecDatas.begin();

  NewInter cmd;
  cmd.mutable_inter()->set_guid(rData.dwGUID);
  cmd.mutable_inter()->set_interid(rData.dwInterID);
  cmd.mutable_inter()->set_source(rData.eSource);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

bool Interlocution::answer(DWORD dwGUID, DWORD dwAnswer)
{
  // get data
  auto v = find_if(m_vecDatas.begin(), m_vecDatas.end(), [dwGUID](const SInterData& rData) -> bool{
    return rData.dwGUID == dwGUID;
  });
  if (v == m_vecDatas.end())
    return false;

  // get config and check correct
  const SInterlocution* pConfig = TableManager::getMe().getInterCFG(v->dwInterID);
  if (pConfig == nullptr)
    return false;

  // inform client result
  Answer cmd;
  cmd.set_guid(v->dwGUID);
  cmd.set_interid(v->dwInterID);
  cmd.set_source(v->eSource);
  cmd.set_answer(dwAnswer);
  cmd.set_correct(pConfig->isCorrect(dwAnswer));
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  // remove interlocution
  ESource eSource = v->eSource;
  m_vecDatas.erase(v);

  // update system
  if (eSource == ESOURCE_QUEST)
    m_pUser->getQuest().onInterlocution(v->dwInterID, cmd.correct());

  // send next locution if exist
  sendInterlocution();

  XLOG << "[Interlocution::answer]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id =" << dwGUID << "typeid =" << cmd.interid() << "answer =" << dwAnswer << XEND;
  return true;
}

