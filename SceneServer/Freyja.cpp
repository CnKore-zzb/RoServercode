#include "Freyja.h"
#include "SceneUser.h"
#include "RecordCmd.pb.h"
#include "SceneNpc.h"
#include "MapConfig.h"
#include "MsgManager.h"
#include "SceneUserManager.h"
#include "SceneNpcManager.h"
#include "ActivityEventManager.h"

Freyja::Freyja(SceneUser *user):
  m_pUser(user)
{
}

Freyja::~Freyja()
{
}

void Freyja::save(Cmd::BlobFreyja *data)
{
  data->Clear();
  data->set_version(1);
  for (auto v = m_vecActiveMapIDs.begin(); v != m_vecActiveMapIDs.end(); ++v)
    data->add_mapid(*v);
  XDBG << "[Freyja-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小:" << data->ByteSize() << XEND;
}

void Freyja::load(const Cmd::BlobFreyja &data)
{
  int version = data.version();

  m_vecActiveMapIDs.clear();
  for (int i = 0; i < data.mapid_size(); ++i)
  {
    const SMapCFG* pBase = MapConfig::getMe().getMapCFG(data.mapid(i));
    if (pBase == nullptr)
      XERR << "[地图传送-读取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "mapid:" << data.mapid(i) << "在Table_Map.txt表中未找到" << XEND;
    m_vecActiveMapIDs.push_back(data.mapid(i));
  }

  if (version >= 2)
  {
  }
}

void Freyja::goToGear(const Cmd::GoToGearUserCmd& cmd)
{
  DWORD mapid = cmd.mapid();
  Cmd::EGoToGearType type =cmd.type();

  const SMapCFG* pBase = MapConfig::getMe().getMapCFG(mapid);
  if (pBase == nullptr)
  {
    XERR << "[地图传送-传送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "传送地图 :" << mapid << "失败,地图不合法" << XEND;
    return;
  }

  if (type == EGoToGearType_Free)
  {
    m_pUser->gomap(mapid, GoMapType::Freyja);
    XLOG << "[地图传送-传送-别人带我传送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "传送地图 :" << mapid <<XEND;
    return;
  }

  // check npc
  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(m_pUser->getVisitNpc());
  if (pNpc == nullptr || pNpc->getCFG() == nullptr)
  {
    XERR << "[地图传送-传送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "传送地图 :" << mapid << "失败,未通过npc传送" << XEND;
    return;
  }
  if (pNpc->getScene() == nullptr || m_pUser->getScene() == nullptr || pNpc->getScene() != m_pUser->getScene())
  {
    XERR << "[地图传送-传送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "传送地图 :" << mapid << "失败,和npc不在同一场景" << XEND;
    return;
  }
  float fDist = ::getDistance(m_pUser->getPos(), pNpc->getPos());
  if (fDist > MiscConfig::getMe().getSystemCFG().fValidPosRadius)
  {
    XERR << "[地图传送-传送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "传送地图 :" << mapid << "失败,距离npc过远" << XEND;
    return;
  }

  if (isVisible(mapid) == false)
  {
    XERR << "[地图传送-传送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "传送地图 :" << mapid << "失败,地图未解锁" << XEND;
    return;
  }

  if (pBase->eTransType == ETransType_MonthCard)
  {
    if (m_pUser->getDeposit().hasMonthCard() == false)
    {
      XERR << "[地图传送-传送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "传送地图 :" << mapid << "失败,未使用月卡" << XEND;
      return;
    }
  }

  if (type == EGoToGearType_Single)
  {
    bool bItemUse = false;
    const SItemMiscCFG& rCFG = MiscConfig::getMe().getItemCFG();
    BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
    if (pMainPack == nullptr)
    {
      XERR << "[地图传送-传送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "传送地图 :" << mapid << "失败,获取" << EPACKTYPE_MAIN << "失败" << XEND;
      return;
    }
    if(MiscConfig::getMe().getFunctionSwitchCFG().dwFreeFreyja != 0)
    {
      m_pUser->getAchieve().onTransWithKPL();
      m_pUser->gomap(mapid, GoMapType::Freyja);
      XLOG << "[地图传送-传送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "传送地图 :" << mapid <<"type"<<type << "活动期间免费传送" << XEND;
      return;
    }
    DWORD dwRob = 0;
    if (ActivityEventManager::getMe().isTransferToMapFree(type, pBase->dwID))
    {
      dwRob = 0;
    }
    else if(pMainPack->checkItemCount(rCFG.dwKapulaMapItem) == true)
    {
      pMainPack->reduceItem(rCFG.dwKapulaMapItem, ESOURCE_KAPULA);
      bItemUse = true;

      MsgParams param;
      param.addNumber(rCFG.dwKapulaMapItem);
      param.addNumber(rCFG.dwKapulaMapItem);
      param.addNumber(1);
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_ITEM_REDUCE, param);
    }
    else
    {
      dwRob = pBase->dwTransMoney;// getTableInt("Money");
    }

    if(bItemUse == false)
    {
      if (m_pUser->checkMoney(EMONEYTYPE_SILVER, dwRob) == false)
        return;
      m_pUser->subMoney(EMONEYTYPE_SILVER, dwRob, ESOURCE_MAPTRANS);
      m_pUser->getAchieve().onKplConsume(dwRob);
    }

    m_pUser->getAchieve().onTransWithKPL();
    m_pUser->gomap(mapid, GoMapType::Freyja);
    m_pUser->stopSendInactiveLog();
    XLOG << "[地图传送-传送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "传送地图 :" << mapid <<"type"<<type << "消耗了" << (bItemUse ? MiscConfig::getMe().getItemCFG().dwKapulaMapItem : dwRob) << (bItemUse ? "道具" : "zeny") << XEND;
    return;
  }
  
  if (type == EGoToGearType_Hand)
  {
    if (!m_pUser->m_oHands.has())
    {
      //TODO sys msg
      XERR << "[地图传送-传送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "传送地图 :" << mapid << "type"<<type << "没有牵手的人" << XEND;
      return;
    }      

    DWORD dwRob = pBase->dwTransMoney* MiscConfig::getMe().getMapTransCFG().fHandRate;    
    if (ActivityEventManager::getMe().isTransferToMapFree(type, pBase->dwID))
      dwRob = 0;
    if (m_pUser->checkMoney(EMONEYTYPE_SILVER, dwRob) == false)
      return;
    m_pUser->subMoney(EMONEYTYPE_SILVER, dwRob, ESOURCE_MAPTRANS);
    m_pUser->getAchieve().onKplConsume(dwRob);
    
    m_pUser->gomap(mapid, GoMapType::Freyja);

    //other user
    QWORD otherId = m_pUser->m_oHands.getOtherID();
    GoToGearUserCmd message;
    message.set_mapid(mapid);
    message.set_type(EGoToGearType_Free);
    PROTOBUF(message, send, len);
    thisServer->forwardCmdToSceneUser(otherId, send, len);
        
    XLOG << "[地图传送-传送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "传送地图 :" << mapid<< "type" << type << "消耗了 zeny" << dwRob << XEND;
    return;
  }

  if (type == EGoToGearType_Team)
  {
    //check
    for (int i = 0; i < cmd.otherids_size(); ++i)
    {
      QWORD otherid = cmd.otherids(i);
      const TeamMemberInfo* pTeamMemInfo = m_pUser->getTeamMember(otherid);
      if (pTeamMemInfo == nullptr)
      {
        XERR << "[地图传送-传送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "传送地图 :" << mapid<< "type" << type << "不是队友" << otherid << XEND;
        return;
      }
    }

    if(MiscConfig::getMe().getFunctionSwitchCFG().dwFreeFreyjaTeam != 0 || ActivityEventManager::getMe().isTransferToMapFree(type, pBase->dwID))
    {
      m_pUser->getAchieve().onTrans();
      m_pUser->gomap(mapid, GoMapType::Freyja);
      for (int i = 0; i < cmd.otherids_size(); ++i)
      {
        GoToGearUserCmd message;
        message.set_mapid(mapid);
        message.set_type(EGoToGearType_Free);
        PROTOBUF(message, send, len);
        thisServer->forwardCmdToSceneUser(cmd.otherids(i), send, len);
      }
      XLOG << "[地图传送-传送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "传送地图 :" << mapid <<"type"<<type << "活动期间免费传送" << XEND;
      m_pUser->stopSendInactiveLog();
      return;
    }

    //check money
    DWORD dwRob = pBase->dwTransMoney * (cmd.otherids_size() + 1);
    if (m_pUser->checkMoney(EMONEYTYPE_SILVER, dwRob) == false)
      return;
    m_pUser->subMoney(EMONEYTYPE_SILVER, dwRob, ESOURCE_MAPTRANS); 
    m_pUser->getAchieve().onKplConsume(dwRob);
    m_pUser->getAchieve().onTrans();

    //go
    m_pUser->gomap(mapid, GoMapType::Freyja);
    for (int i = 0; i < cmd.otherids_size(); ++i)
    {
      GoToGearUserCmd message;
      message.set_mapid(mapid);
      message.set_type(EGoToGearType_Free);
      PROTOBUF(message, send, len);
      thisServer->forwardCmdToSceneUser(cmd.otherids(i), send, len);
    }
    XLOG << "[地图传送-传送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "传送地图 :" << mapid << "type" << type << "消耗了 zeny" << dwRob <<"传送队友" <<cmd.otherids_size() << XEND;
    m_pUser->stopSendInactiveLog();
  }
  return;
}

void Freyja::sendList()
{
  GoToListUserCmd cmd;
  for (auto v = m_vecActiveMapIDs.begin(); v != m_vecActiveMapIDs.end(); ++v)
    cmd.add_mapid(*v);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

bool Freyja::isVisible(DWORD mapid)
{
  return find(m_vecActiveMapIDs.begin(), m_vecActiveMapIDs.end(), mapid) != m_vecActiveMapIDs.end();
}

bool Freyja::addFreyja(DWORD mapid, bool bNotify /*= true*/)
{
  if (isVisible(mapid) == true)
  {
    XERR << "[地图传送-添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "map :" << mapid << "notify :" << bNotify << "失败,已解锁" << XEND;
    return false;
  }

  m_vecActiveMapIDs.push_back(mapid);

  bool bPvpUnlock = false;
  if (isVisible(mapid + 7000) == false)
  {
    const SMapCFG* pBase = MapConfig::getMe().getMapCFG(mapid + 7000);
    if (pBase != nullptr)
    {
      m_vecActiveMapIDs.push_back(mapid + 7000);
      bPvpUnlock = true;
      XLOG << "[地图传送-添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "map :" << (mapid + 7000) << "notify :" << bNotify << XEND;
    }
  }

  if (bNotify)
  {
    NewTransMapCmd cmd;
    cmd.add_mapid(mapid);
    if (bPvpUnlock)
      cmd.add_mapid(mapid + 7000);

    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }

  XLOG << "[地图传送-添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "map :" << mapid << "notify :" << bNotify << XEND;
  return true;
}

