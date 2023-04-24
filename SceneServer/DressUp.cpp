#include "DressUp.h"
#include "SceneUser.h"
#include "Package.h"
#include "SceneNpcManager.h"
#include "SceneBeing.pb.h"
#include "ChatRoomManager.h"

DressUp::DressUp(SceneUser* pUser) : m_pUser(pUser)
{

}

DressUp::~DressUp()
{

}

DWORD DressUp::getDressUpEquipID(EEquipPos pos)
{
  auto it = m_mapDressUpEquip.find(pos);
  if(it != m_mapDressUpEquip.end())
    return it->second;

  return 0;
}

bool DressUp::addDressUpEquipID(EEquipPos pos, DWORD dwItemID)
{
  if(m_mapDressUpEquip[pos] == dwItemID)
    return false;

  if(dwItemID == 0)
  {
    m_mapDressUpEquip[pos] = m_mapDressUpTempEquip[pos];
    m_mapDressUpTempEquip[pos] = dwItemID;
  }
  else
  {
    m_mapDressUpTempEquip[pos] = m_mapDressUpEquip[pos];
    m_mapDressUpEquip[pos] = dwItemID;
  }
  return true;
}

void DressUp::setDressUpStatus(DWORD status, UserStageInfo* pInfo, QueryStageUserCmd& pCmd)
{
  if(m_dwDressUpStatus == status)
    return;

  const SDressStageCFG rCFG = MiscConfig::getMe().getDressStageCFG();
  DWORD oldStatus = m_dwDressUpStatus;
  DWORD oldStageid = m_dwStageID;
  if(oldStatus == 0 && m_pUser->getBuff().haveBuffType(EBUFFTYPE_RIDEWOLF) == true)   //骑狼术
  {
    DWORD dwSkill = rCFG.m_dwRideWolfSkill;
    DWORD dwSkillLv = m_pUser->getSkillLv(dwSkill);
    m_pUser->m_oSkillProcessor.useBuffSkill(m_pUser, m_pUser, dwSkill * ONE_THOUSAND + dwSkillLv, true);
  }

  m_dwDressUpStatus = status;
  m_pUser->setDataMark(EUSERDATATYPE_DRESSUP);
  m_pUser->refreshDataAtonce();

  if(oldStatus == 0)
  {
    if(pInfo->setUsers.size() == 1)
    {
      const xPos pos = rCFG.getEnterPos(m_dwStageID);
      m_pUser->goTo(pos);
    }
    else if(pInfo->setUsers.size() == 2)
    {
      auto itPos = rCFG.m_mapDoubleEnterPos.find(m_dwStageID);
      if(itPos == rCFG.m_mapDoubleEnterPos.end())
      {
        XERR << "[换装舞台-传送]" << "舞台 " << m_dwStageID << "传送点没有找到" << XEND;
        return;
      }

      auto it = pInfo->setUsers.begin();
      if(it != pInfo->setUsers.end() && *it == m_pUser->id)
      {
        const xPos pos = itPos->second.first;
        m_pUser->goTo(pos);
      }
      else
      {
        xPos pos = itPos->second.second;
        m_pUser->goTo(pos);
      }
    }

    m_pUser->m_oBuff.add(rCFG.m_dwUnattackedBuff, m_pUser);

    m_pUser->getUserSceneData().setFollowerID(0);
    m_pUser->breakAllFollowers();
    if (m_pUser->m_oHands.has()) m_pUser->m_oHands.breakup();
    if (m_pUser->getWeaponPet().haveHandCat()) m_pUser->getWeaponPet().breakHand();
    if (m_pUser->getHandNpc().haveHandNpc()) m_pUser->getHandNpc().delHandNpc();
    if (m_pUser->getUserPet().handPet()) m_pUser->getUserPet().breakHand();
    if (m_pUser->getTransform().isInTransform())
    {
      m_pUser->m_oBuff.delBuffByType(EBUFFTYPE_TRANSFORM);
      Attribute* pAttr = m_pUser->getAttribute();
      if (pAttr != nullptr)
        pAttr->setMark();
    }
    if (m_pUser->getUserBeing().getCurBeingID())
    {
      Cmd::BeingOffCmd cmd;
      cmd.set_beingid(m_pUser->getUserBeing().getCurBeingID());
      m_pUser->getUserBeing().handleBeingOffCmd(cmd);
    }
    if(m_pUser->getChatRoom() && m_pUser->getChatRoom()->getRoomID())
      ChatRoomManager::getMe().exitRoom(m_pUser, m_pUser->getChatRoom()->getRoomID());

    EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
    if(pEquipPack != nullptr)
    {
      ItemEquip* pEquip = pEquipPack->getEquip(EEQUIPPOS_BARROW);
      if(pEquip != nullptr)
      {
        bool ret = m_pUser->getPackage().equip(EEQUIPOPER_OFF, EEQUIPPOS_MIN, pEquip->getGUID());
        XLOG << "[换装舞台-手推车]" << m_pUser->accid << m_pUser->id << m_pUser->name << "卸载" << ret << XEND;
      }
    }
  }


  if(pCmd.stageid() != 0)
  {
    PROTOBUF(pCmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }

  if(status == EDRESSUP_WAIT)
  {
    sendWaitUserInfo(pInfo);
  }
  else if(status == EDRESSUP_SHOW)
  {
    initStageAppearance(pInfo);
  }
  else if(status == EDRESSUP_MIN)
  {
    m_dwStageID = 0;
    m_mapDressUpEquip.clear();
    m_mapDressUpTempEquip.clear();
    m_mapUserEquip.clear();

    const xPos pos = rCFG.m_pQuitPos;
    m_pUser->goTo(pos);
    onLeaveStage();
  }

  if(status == EDRESSUP_MIN || oldStatus == 0)
  {
    m_pUser->setDataMark(EUSERDATATYPE_LEFTHAND);
    m_pUser->setDataMark(EUSERDATATYPE_RIGHTHAND);
    m_pUser->setDataMark(EUSERDATATYPE_BACK);
    m_pUser->setDataMark(EUSERDATATYPE_HEAD);
    m_pUser->setDataMark(EUSERDATATYPE_FACE);
    m_pUser->setDataMark(EUSERDATATYPE_TAIL);
    m_pUser->setDataMark(EUSERDATATYPE_MOUNT);
    m_pUser->setDataMark(EUSERDATATYPE_MOUTH);
    m_pUser->setDataMark(EUSERDATATYPE_BODY);
    m_pUser->setDataMark(EUSERDATATYPE_HAIR);
    m_pUser->setDataMark(EUSERDATATYPE_HAIRCOLOR);
    m_pUser->setDataMark(EUSERDATATYPE_EYE);
    m_pUser->setDataMark(EUSERDATATYPE_CLOTHCOLOR);
    m_pUser->refreshDataAtonce();
  }

  XLOG << "[换装舞台-状态]" << m_pUser->accid << m_pUser->id << m_pUser->name << " from: " << oldStatus << " to: " << m_dwDressUpStatus << oldStageid << XEND;
}

void DressUp::sendWaitUserInfo(const UserStageInfo* pInfo)
{
  DressUpStageUserCmd cmd;
  for(auto &s : pInfo->setUsers)
  {
    if(s != m_pUser->id)
      cmd.add_userid(s);
  }

  for(auto it = pInfo->mapAttr.begin(); it != pInfo->mapAttr.end(); ++it)
  {
    StageUserDataType* pData = cmd.add_datas();
    if(pData != nullptr)
    {
      pData->set_type(it->first);
      pData->set_value(it->second);
    }
  }

  cmd.set_stageid(m_dwStageID);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
#ifdef _DEBUG
  XLOG <<  "[换装舞台-队伍], 排队玩家" << m_pUser->name << m_pUser->id << "消息内容: " << cmd.ShortDebugString() << XEND;
#endif
}

void DressUp::initStageAppearance(const UserStageInfo* pInfo)
{
  SceneNpc* pNpc = DressUpStageMgr::getMe().getStageNpc(m_dwStageID);
  if(pNpc == nullptr)
    return;

  for(auto it = pInfo->mapAttr.begin(); it != pInfo->mapAttr.end(); ++it)
  {
    DressUpStageMgr::getMe().changeStageAppearance(m_pUser, m_dwStageID, it->first, it->second, pNpc);
  }

  XLOG <<  "[换装舞台-消息], 舞台玩家" << m_pUser->name << m_pUser->id << m_pUser->accid << "初始化舞台信息 " << m_dwStageID << XEND;
}

void DressUp::onLeaveScene()
{
  if(m_dwDressUpStatus == 0)
    return;

  DressUpStageMgr::getMe().leaveDressStage(m_pUser);
  onLeaveStage();
}

void DressUp::onLeaveStage()
{
  const SDressStageCFG rCFG = MiscConfig::getMe().getDressStageCFG();
  if(m_pUser->getScene() && m_pUser->getScene()->getMapID() != rCFG.m_dwStaticMap)
    return;

  xPos dest;
  if (m_pUser->getScene()->getRandPos(rCFG.m_pQuitPos, rCFG.m_dwMateRange, dest))
  {
    m_pUser->m_oGingerBread.resetGingerPos(dest);
  }

  //equipOnAll();
  if(m_pUser->m_oBuff.haveBuff(rCFG.m_dwUnattackedBuff))
  {
    m_pUser->m_oBuff.del(rCFG.m_dwUnattackedBuff, m_pUser);
    m_pUser->getBuff().update(xTime::getCurMSec());
    m_pUser->getAttribute()->updateAttribute();
  }
}

void DressUp::equipOffAll()
{
  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
  if(pEquipPack == nullptr)
    return;

  for (int i = EEQUIPPOS_MIN + 1; i < EEQUIPPOS_MAX; ++i)
  {
    ItemEquip* pEquip = pEquipPack->getEquip(static_cast<EEquipPos>(i));
    if (pEquip != nullptr && pEquip->canEquip(m_pUser->getUserSceneData().getProfession()) == true)
    {
      EEquipPos ePos = static_cast<EEquipPos>(pEquip->getIndex());
      DWORD dwItemID = pEquip->getTypeID();
      bool ret = m_pUser->getPackage().equip(EEQUIPOPER_OFF, EEQUIPPOS_MIN, pEquip->getGUID());
      if(ret)
      {
        m_mapUserEquip.emplace(ePos, pEquip->getGUID());
        addDressUpEquipID(ePos, pEquip->getTypeID());
      }
#ifdef _DEBUG
      XLOG <<  "[换装舞台-装备] 脱掉, 玩家" << m_pUser->name << m_pUser->id << m_pUser->accid << "装备" << dwItemID << ret << XEND;
#endif
    }
  }
}

void DressUp::equipOnAll()
{
  if(m_mapUserEquip.empty() == true)
    return;

  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
  if(pEquipPack == nullptr)
    return;

  for(auto &s : m_mapUserEquip)
  {
    m_pUser->getPackage().equip(EEQUIPOPER_ON, s.first, s.second);
#ifdef _DEBUG
    XLOG <<  "[换装舞台-装备] 穿上, 玩家" << m_pUser->name << m_pUser->id << m_pUser->accid << "装备" << s.first << s.second << XEND;
#endif
  }

  m_mapUserEquip.clear();
}
