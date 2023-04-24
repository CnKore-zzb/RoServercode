#include "Portrait.h"
#include "SceneUser.h"
#include "RecordCmd.pb.h"

// portrait
Portrait::Portrait(SceneUser* pUser) : m_pUser(pUser)
{

}

Portrait::~Portrait()
{

}

DWORD Portrait::getCurPortrait(bool isReal /*=false*/) const
{
  if (isReal)
    return m_dwCurPortrait;
  if (!m_pUser)
    return m_dwCurPortrait;
  if (m_pUser->getBuff().hasPartTransform(EUSERDATATYPE_PORTRAIT))
  {
    return m_pUser->getBuff().getPartTransform(EUSERDATATYPE_PORTRAIT);
  }
  return m_dwCurPortrait;
}

bool Portrait::load(const BlobPortrait& acc_data, const BlobPortrait& char_data)
{
  m_dwCurPortrait = char_data.curportrait();
  m_dwCurFrame = char_data.curframe();

  m_vecUnlockPortrait.clear();
  for (int i = 0; i < acc_data.unlockportrait_size(); ++i)
    m_vecUnlockPortrait.push_back(acc_data.unlockportrait(i));
  m_vecUnlockFrame.clear();
  for (int i = 0; i < acc_data.unlockframe_size(); ++i)
    m_vecUnlockFrame.push_back(acc_data.unlockframe(i));
  return true;
}

bool Portrait::save(BlobPortrait* acc_data, BlobPortrait* char_data)
{
  char_data->set_curportrait(m_dwCurPortrait);
  char_data->set_curframe(m_dwCurFrame);

  acc_data->clear_unlockportrait();
  for (auto v = m_vecUnlockPortrait.begin(); v != m_vecUnlockPortrait.end(); ++v)
    acc_data->add_unlockportrait(*v);
  acc_data->clear_unlockframe();
  for (auto v = m_vecUnlockFrame.begin(); v != m_vecUnlockFrame.end(); ++v)
    acc_data->add_unlockframe(*v);

  XDBG << "[头像相框-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小 acc :" << acc_data->ByteSize() << "char :" << char_data->ByteSize() << XEND;
  return true;
}

DWORD Portrait::getPortraitCount(EPortraitType eType) const
{
  DWORD dwCount = 0;
  for (auto &v : m_vecUnlockPortrait)
  {
    const SPortraitCFG* pCFG = PortraitConfig::getMe().getPortraitCFG(v);
    if (pCFG != nullptr && pCFG->eType == eType)
      ++dwCount;
  }
  return dwCount;
}

bool Portrait::checkAddItems(DWORD id)
{
  const SPortraitCFG* pCFG = PortraitConfig::getMe().getPortraitCFG(id);
  if (pCFG == nullptr)
    return false;

  if (pCFG->eType == EPORTRAITTYPE_USERPORTRAIT || pCFG->eType == EPORTRAITTYPE_PETPORTRAIT)
  {
    auto v = find(m_vecUnlockPortrait.begin(), m_vecUnlockPortrait.end(), pCFG->dwID);
    if (v != m_vecUnlockPortrait.end())
      return false;

    if (pCFG->eGender != EGENDER_MIN)
    {
      if (m_pUser->getUserSceneData().getGender() != pCFG->eGender)
        return false;
    }
  }
  else if (pCFG->eType == EPORTRAITTYPE_FRAME)
  {
    auto v = find(m_vecUnlockFrame.begin(), m_vecUnlockFrame.end(), pCFG->dwID);
    if (v != m_vecUnlockFrame.end())
      return false;
  }
  else
    return false;

  return true;
}

bool Portrait::addNewItems(DWORD id, bool bNotify /*= true*/)
{
  if (checkAddItems(id) == false)
    return false;

  const SPortraitCFG* pCFG = PortraitConfig::getMe().getPortraitCFG(id);
  if (pCFG == nullptr)
    return false;

  if (pCFG->eType == EPORTRAITTYPE_USERPORTRAIT || pCFG->eType == EPORTRAITTYPE_PETPORTRAIT)
    addNewPortrait(pCFG, bNotify);
  else if (pCFG->eType == EPORTRAITTYPE_FRAME)
    addNewFrame(pCFG, bNotify);
  else
    return false;

  m_pUser->getTip().onPortrait(id);
  ItemInfo oInfo;
  oInfo.set_id(id);
  m_pUser->getEvent().onItemAdd(oInfo);
  return true;
}

void Portrait::sendAllUnlockItems()
{
  QueryPortraitListUserCmd cmd;
  for (auto v = m_vecUnlockPortrait.begin(); v != m_vecUnlockPortrait.end(); ++v)
    cmd.add_portrait(*v);
  /*for (auto v = m_vecUnlockFrame.begin(); v != m_vecUnlockFrame.end(); ++v)
    cmd.add_frame(*v);*/

  if (cmd.portrait_size() > 0)// || cmd.frame_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

void Portrait::refreshEnableItems()
{

}

bool Portrait::usePortrait(DWORD id, bool bNotify /*= true*/)
{
  // check same id
  if (m_dwCurPortrait == id)
    return false;

  // check config
  if (id != 0)
  {
    const SPortraitCFG* pCFG = PortraitConfig::getMe().getPortraitCFG(id);
    if (pCFG == nullptr)
      return false;

    // check id valid
    auto v = find(m_vecUnlockPortrait.begin(), m_vecUnlockPortrait.end(), id);
    if (v == m_vecUnlockPortrait.end())
      return false;
  }

  // set NEW portrait
  m_dwCurPortrait = id;

  // save data
  m_pUser->setDataMark(EUSERDATATYPE_PORTRAIT);
  m_pUser->refreshDataAtonce();

  // inform client
  if (bNotify)
  {
    UsePortrait cmd;
    cmd.set_id(id);
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }

  XLOG << "[头像相框-使用头像]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "使用id :" << id << "头像" << XEND;
  return true;
}

bool Portrait::useFrame(DWORD id, bool bNotify /*= true*/)
{
  // check same id
  if (m_dwCurFrame == id)
    return false;

  // check config
  const SPortraitCFG* pCFG = PortraitConfig::getMe().getPortraitCFG(id);
  if (pCFG == nullptr)
    return false;

  // check id valid
  auto v = find(m_vecUnlockFrame.begin(), m_vecUnlockFrame.end(), id);
  if (v == m_vecUnlockFrame.end())
    return false;

  // set NEW portrait
  m_dwCurFrame = id;

  // save data
  m_pUser->setDataMark(EUSERDATATYPE_FRAME);
  m_pUser->refreshDataAtonce();

  // inform client
  if (bNotify)
  {
    UseFrame cmd;
    cmd.set_id(id);
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }

  XLOG << "[头像相框-使用相框]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "使用id :" << id << "相框" << XEND;
  return true;
}

bool Portrait::patch1()
{
  m_vecUnlockPortrait.clear();
  m_dwCurPortrait = 0;
  return true;
}

bool Portrait::addNewPortrait(const SPortraitCFG* pCFG, bool bNotify)
{
  if (pCFG == nullptr)
    return false;

  auto v = find(m_vecUnlockPortrait.begin(), m_vecUnlockPortrait.end(), pCFG->dwID);
  if (v != m_vecUnlockPortrait.end())
    return false;

  if (pCFG->eGender != EGENDER_MIN)
  {
    if (m_pUser->getUserSceneData().getGender() != pCFG->eGender)
      return false;
  }

  m_vecUnlockPortrait.push_back(pCFG->dwID);
  m_pUser->getAchieve().onPortrait();

  if (bNotify)
  {
    NewPortraitFrame cmd;
    cmd.add_portrait(pCFG->dwID);
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }

  XLOG << "[头像相框-头像添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "获得 id :" << pCFG->dwID << "头像" << XEND;
  return true;
}

bool Portrait::addNewFrame(const SPortraitCFG* pCFG, bool bNotify)
{
  if (pCFG == nullptr)
    return false;

  auto v = find(m_vecUnlockFrame.begin(), m_vecUnlockFrame.end(), pCFG->dwID);
  if (v != m_vecUnlockFrame.end())
    return false;

  m_vecUnlockFrame.push_back(pCFG->dwID);

  if (bNotify)
  {
    NewPortraitFrame cmd;
    cmd.add_frame(pCFG->dwID);
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }

  XLOG << "[头像相框-相框添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "获得 id :" << pCFG->dwID << "相框" << XEND;
  return true;
}

