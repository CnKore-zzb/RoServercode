/**
 * @file Hair.cpp
 * @brief 
 * @author gengshengjie, gengshengjie@xindong.com
 * @version v1
 * @date 2015-09-02
 */
#include "Hair.h"
#include "SceneUser.h"
#include "TableManager.h"
#include "RecordCmd.pb.h"
#include "Menu.h"

Hair::Hair(SceneUser* pUser) : m_pUser(pUser)
{

}

Hair::~Hair()
{

}

bool Hair::load(const BlobHair& data)
{
  m_dwCurHair = data.curhair();
  m_dwCurColor = data.curcolor();

  m_setUnlockHair.clear();
  for (int i = 0; i <data.unlockhair_size(); ++i)
    m_setUnlockHair.insert(data.unlockhair(i));

  if (m_setUnlockHair.empty() && m_dwCurHair != 0)
    m_setUnlockHair.insert(m_dwCurHair);

  return true;
}

bool Hair::save(BlobHair* data)
{
  if (data == nullptr)
    return false;

  data->set_curhair(m_dwCurHair);
  data->set_curcolor(m_dwCurColor);

  data->clear_unlockhair();
  for (auto v = m_setUnlockHair.begin(); v != m_setUnlockHair.end(); ++v)
    data->add_unlockhair(*v);

  XDBG << "[发型-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小 :" << data->ByteSize() << XEND;
  return true;
}

void Hair::sendAllUnlockHairs()
{
  DressingListUserCmd cmd;
  cmd.set_type(EDRESSTYPE_HAIR);
  for (auto v = m_setUnlockHair.begin(); v != m_setUnlockHair.end(); ++v)
    cmd.add_dressids(*v);
  if (cmd.dressids_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

bool Hair::checkAddHair(DWORD id)
{
  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(id);
  if (pCFG == nullptr)
    return false;

  auto s = m_setUnlockHair.find(pCFG->dwHairID);
  if (s != m_setUnlockHair.end())
    return false; //提示已添加
  if (pCFG->eItemType != EITEMTYPE_HAIR)
  {
    if (pCFG->eItemType == EITEMTYPE_HAIR_MALE && m_pUser->getUserSceneData().getGender() != EGENDER_MALE)
      return false;
    if (pCFG->eItemType == EITEMTYPE_HAIR_FEMALE && m_pUser->getUserSceneData().getGender() != EGENDER_FEMALE)
      return false;
  }

  return true;
}

bool Hair::addNewHair(DWORD id)
{
  if (checkAddHair(id) == false)
  {
    XERR << "[玩家-发型添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加发型" << id << "失败" << XEND;
    return false;
  }
  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(id);
  if (pCFG == nullptr)
  {
    XERR << "[玩家-发型添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加发型" << id << "失败" << XEND;
    return false;
  }

  m_setUnlockHair.insert(pCFG->dwHairID);
  m_pUser->getAchieve().onHair(false);

  //notify nine
  NewDressing cmd;
  cmd.set_type(EDRESSTYPE_HAIR);
  cmd.add_dressids(pCFG->dwHairID);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[发型-添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "itemid :" << id << "hairid :" << pCFG->dwHairID << XEND;
  return true;
}

bool Hair::useColor(DWORD id, bool bRealUse /*= true*/)
{
  //check same
  if (id == m_dwCurColor && bRealUse == true)
    return false;

  //check config
  const SHairColor* pCFG = TableManager::getMe().getHairColorCFG(id);
  if (pCFG == nullptr && id != 0)
    return false;

  if(bRealUse == false)
  {
    if(id == m_pairHairColor.first)
      return false;
    m_pairHairColor.second = m_pairHairColor.first;
    m_pairHairColor.first = id;
  }
  /*//check stuff
  for (auto p = pCFG->vecStuff.begin(); p != pCFG->vecStuff.end(); ++p)
  {
    bool enough = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN)->checkItemCount(p->first, p->second);
    if (!enough)
      return false;
  }

  //consume stuff
  for (auto p = pCFG->vecStuff.begin(); p != pCFG->vecStuff.end(); ++p)
  {
    m_pUser->getPackage().getPackage(EPACKTYPE_MAIN)->reduceItem(p->first ,ESOURCE_HAIR, p->second);
  }*/

  //set NEW color
  if(bRealUse == true)
    m_dwCurColor = id;

  //save data
  m_pUser->setDataMark(EUSERDATATYPE_HAIRCOLOR);
  m_pUser->refreshDataAtonce();
  if(bRealUse == true)
    m_pUser->getServant().onFinishEvent(ETRIGGER_HAIR);

  //inform client to nine
  UseDressing cmd;
  cmd.set_type(EDRESSTYPE_HAIRCOLOR);
  cmd.set_id(getCurHairColor());
  cmd.set_charid(m_pUser->id);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToNine(send, len);

  XLOG << "[发型-设置颜色]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << id << bRealUse << XEND;
  return true;
}

bool Hair::useHair(DWORD id, bool bRealUse /*= true*/)
{
  if(bRealUse == true)
  {
    //check config
    const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(id);
    if (pCFG == nullptr)
    {
      XERR << "[发型-设置发型]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "itemid :" << id << "失败,未在 Table_Item.txt 表中找到" << XEND;
      return false;
    }

    //check same
    if (pCFG->dwHairID == m_dwCurHair)
    {
      XERR << "[发型-设置发型]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "itemid :" << id << "hairid :" << pCFG->dwHairID << "失败,当前已是该发型" << XEND;
      return false;
    }

    //check ownership
    auto v = find(m_setUnlockHair.begin(), m_setUnlockHair.end(), pCFG->dwHairID);
    if (v == m_setUnlockHair.end())
    {
      XERR << "[发型-设置发型]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "itemid :" << id << "hairid :" << pCFG->dwHairID << "失败,该发型未解锁" << XEND;
      return false; //提示未获得
    }

    //set NEW hair
    m_dwCurHair = pCFG->dwHairID;
    m_pUser->getAchieve().onHair(true);
    m_pUser->getServant().onFinishEvent(ETRIGGER_HAIR);
  }
  else
  {
    const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(id);
    if (pCFG == nullptr || pCFG->dwHairID == 0)
    {
      XERR << "[发型-设置发型]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "itemid :" << id << "失败,未在 Table_Item.txt 表中找到" << XEND;
      return false;
    }

    if(m_pairHair.first == pCFG->dwHairID)
      return false;

    m_pairHair.second = m_pairHair.first;
    m_pairHair.first = pCFG->dwHairID;
  }

  //save data
  m_pUser->setDataMark(EUSERDATATYPE_HAIR);
  m_pUser->refreshDataAtonce();

  //inform client to nine
  UseDressing cmd;
  cmd.set_type(EDRESSTYPE_HAIR);
  cmd.set_id(getCurHair());
  cmd.set_charid(m_pUser->id);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToNine(send, len);

  XLOG << "[发型-设置发型]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "itemid :" << id << "hairid :" << getCurHair()
    << bRealUse << XEND;
  return true;
}

bool Hair::useHairFree(DWORD id)
{
  //check config
  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(id);
  if (pCFG == nullptr)
    return false;

  //check same
  if (pCFG->dwHairID == m_dwCurHair)
    return false;

  //get free hair
  auto v = find(m_setUnlockHair.begin(), m_setUnlockHair.end(), pCFG->dwHairID);
  if (v == m_setUnlockHair.end())
    m_setUnlockHair.insert(id);

  //set NEW hair
  m_dwCurHair = pCFG->dwHairID;

  //save data
  m_pUser->setDataMark(EUSERDATATYPE_HAIR);
  m_pUser->refreshDataAtonce();

  //inform nine
  //UseHair cmd;
  //cmd.set_id(id);
  //PROTOBUF(cmd, send, len);
  //m_pUser->sendMeToNine(send, len);

  XLOG << "[发型-设置免费发型]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "itemid :" << id << "hairid :" << pCFG->dwHairID << XEND;
  return true;
}

bool Hair::useColorFree(DWORD id)
{
  //check same
  if (id == m_dwCurColor)
    return false;

  //check config
  const SHairColor* pCFG = TableManager::getMe().getHairColorCFG(id);
  if (pCFG == nullptr)
    return false;

  //set NEW hair
  m_dwCurColor = id;

  //save data
  m_pUser->setDataMark(EUSERDATATYPE_HAIRCOLOR);
  m_pUser->refreshDataAtonce();

  XLOG << "[发型-设置免费颜色]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << id << XEND;
  return true;
}

bool Hair::hasHair(DWORD id) const
{
  return m_setUnlockHair.find(id) != m_setUnlockHair.end();
}

void Hair::resetHair()
{
  TSetDWORD setOriHair(m_setUnlockHair);

  const TVecMenuCFG* pVecCFG = MenuConfig::getMe().getEventMenuList(EMENUEVENT_UNLOCKHAIR);
  if (pVecCFG == nullptr)
  {
    XERR << "[发型-重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置失败,原在 Table_Menu.txt 表中找到解锁发型事件" << XEND;
    return;
  }

  m_setUnlockHair.clear();
  const SNewRoleCFG& rCFG = MiscConfig::getMe().getNewRoleCFG();

  for (auto v = rCFG.vecHair.begin(); v != rCFG.vecHair.end(); ++v)
  {
    const SItemCFG* pCFG = ItemConfig::getMe().getHairCFG(*v);
    if (pCFG == nullptr)
    {
      XERR << "[发型-重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置成功,添加NewRole默认发型" << *v << "失败, 未在Table_Item.txt表中找到" << XEND;
      continue;
    }
    if (checkAddHair(pCFG->dwTypeID) == true)
      addNewHair(pCFG->dwTypeID);
  }

  UserSceneData& rSceneData = m_pUser->getUserSceneData();
  Menu& rMenu = m_pUser->getMenu();
  const TVecGenderReward& vecGender = ItemConfig::getMe().getGenderRewardList();
  for (auto &v : vecGender)
  {
    if (rSceneData.getGender() == EGENDER_MALE && rMenu.isOpen(v.second) == true)
    {
      rMenu.close(v.second);
      rMenu.open(v.first);
    }
    else if (rSceneData.getGender() == EGENDER_FEMALE && rMenu.isOpen(v.first) == true)
    {
      rMenu.close(v.first);
      rMenu.open(v.second);
    }
  }

  for (auto &v : *pVecCFG)
  {
    if (rMenu.isOpen(v.id) == false)
      continue;

    for (auto &hair : v.stEvent.vecParams)
    {
      const SItemCFG* pItemCFG = ItemConfig::getMe().getHairCFG(hair);
      if (pItemCFG == nullptr)
      {
        XERR << "[发型-重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置menuid :" << v.id << "hairid :" << hair << "失败,未在Table_Hair.txt表中找到" << XEND;
        continue;
      }
      if (checkAddHair(pItemCFG->dwTypeID) == true)
        addNewHair(pItemCFG->dwTypeID);
    }
    XLOG << "[发型-重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置menuid :" << v.id << "发型解锁成功" << XEND;
  }

  m_dwCurHair = 0;
  DWORD dwDefaultHair = rCFG.getDefaultHair(m_pUser->getUserSceneData().getGender());
  if (useHair(dwDefaultHair) == false)
    XERR << "[发型-重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置成功,设置默认发型 :" << dwDefaultHair << "失败" << XEND;

  sendAllUnlockHairs();
  XLOG << "[发型-重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "重置成功,原 :" << setOriHair << "现 :" << m_setUnlockHair << XEND;
}

DWORD Hair::getCurHairColor()
{
  if(m_pUser->getDressUp().getDressUpStatus() != 0 && m_pairHairColor.first != 0)
    return m_pairHairColor.first;
  if (m_pUser->getTransform().isInTransform())
    return m_pUser->m_oBuff.getTransform(EUSERDATATYPE_HAIRCOLOR);
  if (m_pUser->m_oBuff.hasPartTransform(EUSERDATATYPE_HAIRCOLOR))
    return m_pUser->m_oBuff.getPartTransform(EUSERDATATYPE_HAIRCOLOR);

  return m_dwCurColor;
}

DWORD Hair::getCurHair(bool bReal /*= false*/)
{
  if(m_pUser->getDressUp().getDressUpStatus() != 0 && m_pairHair.first != 0)
    return m_pairHair.first;

  if (bReal)
    return m_dwCurHair;

  if (m_pUser->getTransform().isInTransform())
    return m_pUser->m_oBuff.getTransform(EUSERDATATYPE_HAIR);
  if (m_pUser->m_oBuff.hasPartTransform(EUSERDATATYPE_HAIR))
    return m_pUser->m_oBuff.getPartTransform(EUSERDATATYPE_HAIR);

  return m_dwCurHair;
}

