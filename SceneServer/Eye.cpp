#include "Eye.h"
#include "SceneUser.h"
#include "Menu.h"

Eye::Eye(SceneUser* pUser) : m_pUser(pUser)
{

}

Eye::~Eye()
{

}

bool Eye::load(const BlobEye& rBlob)
{
  m_setUnlockIDs.clear();

  m_dwCurID = rBlob.curid();
  for (int i = 0; i < rBlob.unlockids_size(); ++i)
    m_setUnlockIDs.insert(rBlob.unlockids(i));

  initDefault();
  return true;
}

bool Eye::save(BlobEye* pBlob)
{
  if (pBlob == nullptr)
    return false;

  pBlob->Clear();
  pBlob->set_curid(m_dwCurID);
  for (auto &s : m_setUnlockIDs)
    pBlob->add_unlockids(s);

  XDBG << "[美瞳-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小 :" << pBlob->ByteSize() << XEND;
  return true;
}

DWORD Eye::getCurID(bool bReal /*= false*/) const
{
  if(m_pUser->getDressUp().getDressUpStatus() != 0 && m_pairDressUpID.first != 0)
    return m_pairDressUpID.first;

  if (bReal)
    return m_dwCurID;

  if (m_pUser->getTransform().isInTransform())
    return m_pUser->m_oBuff.getTransform(EUSERDATATYPE_EYE);
  if (m_pUser->m_oBuff.hasPartTransform(EUSERDATATYPE_EYE))
    return m_pUser->m_oBuff.getPartTransform(EUSERDATATYPE_EYE);
  return m_dwCurID;
}

void Eye::sendUnlockList() const
{
  DressingListUserCmd cmd;
  cmd.set_type(EDRESSTYPE_EYE);
  for (auto &s : m_setUnlockIDs)
    cmd.add_dressids(s);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XDBG << "[美瞳-同步]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "同步数据" << cmd.ShortDebugString() << XEND;
}

void Eye::resetEye()
{
  TSetDWORD setOriIDs(m_setUnlockIDs);

  const TVecMenuCFG* pVecCFG = MenuConfig::getMe().getEventMenuList(EMENUEVENT_UNLOCKEYE);
  if (pVecCFG == nullptr)
  {
    XERR << "[美瞳-重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置失败,在 Table_Menu.txt 表中找到解锁美瞳事件" << XEND;
    return;
  }

  m_setUnlockIDs.clear();

  const SNewRoleCFG& rCFG = MiscConfig::getMe().getNewRoleCFG();
  for (auto v = rCFG.vecEye.begin(); v != rCFG.vecEye.end(); ++v)
  {
    const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(*v);
    if (pCFG == nullptr)
    {
      XERR << "[美瞳-重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置成功,添加NewRole默认美瞳" << *v << "失败,未在Table_Item.txt表中找到" << XEND;
      continue;
    }
    if (checkAddEye(*v) == true)
      addNewEye(*v);
  }

  for (auto &v : *pVecCFG)
  {
    if (m_pUser->getMenu().isOpen(v.id) == false)
      continue;
    for (auto &eye : v.stEvent.vecParams)
    {
      const SItemCFG* pItemCFG = ItemConfig::getMe().getItemCFG(eye);
      if (pItemCFG == nullptr)
      {
        XERR << "[美瞳-重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置menuid :" << v.id << "eyeid :" << eye << "未在Table_Item.txt表中找到" << XEND;
        continue;
      }
      if (checkAddEye(pItemCFG->dwTypeID) == true)
        addNewEye(pItemCFG->dwTypeID);
    }
    XLOG << "[美瞳-重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置menuid :" << v.id << "美瞳解锁成功" << XEND;
  }

  DWORD dwDefaultEye = rCFG.getDefaultEye(m_pUser->getUserSceneData().getGender());
  if (hasEye(dwDefaultEye) == false)
  {
    DWORD dwOri = dwDefaultEye;
    dwDefaultEye = m_setUnlockIDs.empty() == true ? 0 : *m_setUnlockIDs.begin();
    XERR << "[美瞳-重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置成功,未包含默认美瞳" << dwOri << "刷新默认美瞳为" << dwDefaultEye << XEND;
  }
  m_dwCurID = 0;
  if (useEye(dwDefaultEye) == false)
    XERR << "[美瞳-重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置成功,设置默认发型" << dwDefaultEye << "失败" << XEND;

  sendUnlockList();
  XLOG << "[美瞳-重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置成功,原 :" << setOriIDs << "现 :" << m_setUnlockIDs << XEND;
}

bool Eye::checkAddEye(DWORD dwID)
{
  if (hasEye(dwID) == true)
    return false;

  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(dwID);
  if (pCFG == nullptr)
    return false;
  if (pCFG->eItemType != EITEMTYPE_EYE_MALE && pCFG->eItemType != EITEMTYPE_EYE_FEMALE)
    return false;

  if (pCFG->eItemType == EITEMTYPE_EYE_MALE && m_pUser->getUserSceneData().getGender() != EGENDER_MALE)
    return false;
  if (pCFG->eItemType == EITEMTYPE_EYE_FEMALE && m_pUser->getUserSceneData().getGender() != EGENDER_FEMALE)
    return false;

  return true;
}

bool Eye::addNewEye(DWORD dwID)
{
  if (checkAddEye(dwID) == false)
  {
    XERR << "[美瞳-添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加" << dwID << "失败" << XEND;
    return false;
  }

  m_setUnlockIDs.insert(dwID);

  NewDressing cmd;
  cmd.set_type(EDRESSTYPE_EYE);
  cmd.add_dressids(dwID);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[美瞳-添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加" << dwID << "成功" << XEND;
  return true;
}

bool Eye::useEye(DWORD dwID, bool bRealUse /*= true*/)
{
  if(bRealUse == false)
  {
    if(m_pairDressUpID.first == dwID)
      return false;
    m_pairDressUpID.second = m_pairDressUpID.first;
    m_pairDressUpID.first = dwID;
  }
  if (m_dwCurID == dwID && bRealUse == true)
  {
    XERR << "[美瞳-使用]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "使用" << dwID << "失败,当前已经是该美瞳" << XEND;
    return false;
  }
  if (hasEye(dwID) == false && bRealUse == true)
  {
    XERR << "[美瞳-使用]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "使用" << dwID << "失败,未解锁" << XEND;
    return false;
  }

  if(bRealUse == true)
    m_dwCurID = dwID;

  UseDressing cmd;
  cmd.set_type(EDRESSTYPE_EYE);
  cmd.set_id(getCurID());
  cmd.set_charid(m_pUser->id);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToNine(send, len);

  m_pUser->setDataMark(EUSERDATATYPE_EYE);
  m_pUser->refreshDataAtonce();
  if(bRealUse == true)
    m_pUser->getServant().onFinishEvent(ETRIGGER_EYE);

  XLOG << "[美瞳-使用]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "使用" << dwID << bRealUse << "成功" << XEND;
  return true;
}

void Eye::initDefault()
{
  const SNewRoleCFG& rCFG = MiscConfig::getMe().getNewRoleCFG();
  for (auto &v : rCFG.vecEye)
    m_setUnlockIDs.insert(v);

  if (m_dwCurID == 0 && m_pUser->getRoleBaseCFG() != nullptr)
  {
    if (m_pUser->getUserSceneData().getGender() == EGENDER_MALE)
      m_dwCurID = m_pUser->getRoleBaseCFG()->maleEye;
    else if (m_pUser->getUserSceneData().getGender() == EGENDER_FEMALE)
      m_dwCurID = m_pUser->getRoleBaseCFG()->femaleEye;
  }
}

