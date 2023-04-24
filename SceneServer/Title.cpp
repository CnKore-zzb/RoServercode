#include "Title.h"
#include "SceneUser.h"
#include "PlatLogManager.h"
#include "SceneServer.h"
#include "MsgManager.h"
#include "MailManager.h"
#include "Menu.h"

Title::Title(SceneUser* pUser) : m_pUser(pUser)
{
}

Title::~Title()
{
}

bool Title::load(const BlobTitle& acc_data, const BlobTitle& char_data)
{
  m_mapTitle.clear();
  for (int i = 0; i < acc_data.datas_size(); ++i)
  {
    const TitleData titleData = acc_data.datas(i);

    if (ItemManager::getMe().getItemCFG(titleData.id()) == nullptr)
    {
      XERR << "[称号-添加] 配置表中未查到 " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "TitleID " << titleData.id() << XEND;
      continue;
    }

    auto iter = m_mapTitle.find(titleData.title_type());
    if (iter == m_mapTitle.end())
    {
      iter=m_mapTitle.insert(std::make_pair(titleData.title_type(), std::vector<DWORD>())).first;
    }
    iter->second.push_back(titleData.id());
  }
  curAhieveTitle = char_data.curachievetitle();

  m_pUser->setCollectMark(ECOLLECTTYPE_ACHIEVEMENT);

  XLOG << "[称号-读取保存] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << " curAhieveTitle: " << curAhieveTitle << XEND;
  return true;
}

bool Title::save(BlobTitle* acc_data, BlobTitle* char_data)
{
  if (acc_data == nullptr || char_data == nullptr)
    return false;

  for (auto it = m_mapTitle.begin(); it != m_mapTitle.end(); ++it)
  {
    for (auto subIt = it->second.begin(); subIt != it->second.end(); ++subIt)
    {
      TitleData*pTitleData = acc_data->add_datas();
      pTitleData->set_title_type(it->first);
      pTitleData->set_id(*subIt);
    }
  }
  char_data->set_curachievetitle(curAhieveTitle);

  XDBG << "[称号-保存] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << " 数据大小 : " << acc_data->ByteSize() << XEND;
  return true;
}

void Title::sendTitleData()
{
  AllTitle cmd;
  for (auto it = m_mapTitle.begin(); it != m_mapTitle.end(); ++it)
  {
    for (auto subIt = it->second.begin(); subIt != it->second.end(); ++subIt)
    {
      TitleData* pData = cmd.add_title_datas();
      pData->set_title_type(it->first);
      pData->set_id(*subIt);
    }
  }
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);  
}

bool Title::checkAddTitle(DWORD titleId)
{
  const SItemCFG* pBase = ItemManager::getMe().getItemCFG(titleId);
  if (pBase == nullptr)
  {
    XERR << "[称号-添加] 配置表错误 " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << XEND;
    return false;
  }

  ETitleType type = pBase->eTitleType;
  if (!canAdd(type, titleId))
  {
    return false;
  }
  return true;
}

bool Title::addTitle(DWORD titleId)
{
  const SItemCFG* pBase = ItemManager::getMe().getItemCFG(titleId);
  if (pBase == nullptr)
  {
    XERR << "[称号-添加] 配置表错误 " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name <<  XEND;
    return false;
  }

  ETitleType type = pBase->eTitleType;
  if (!canAdd(type, titleId))
  {
    return false;
  }

  auto iter = m_mapTitle.find(type);
  if (iter == m_mapTitle.end())
  {
    iter=m_mapTitle.insert(std::make_pair(type, std::vector<DWORD>())).first;
  }
  iter->second.push_back(titleId);
  bool needchange = false;
  if(type == ETITLE_TYPE_MANNUAL)
  {
    m_pUser->setDataMark(EUSERDATATYPE_CUR_TITLE);
    m_pUser->setCollectMark(ECOLLECTTYPE_ACHIEVEMENT);
  }
  else if(type == ETITLE_TYPE_ACHIEVEMENT || type == ETITLE_TYPE_ACHIEVEMENT_ORDER)
  {
    DWORD count = getTitleNum(ETITLE_TYPE_ACHIEVEMENT) + getTitleNum(ETITLE_TYPE_ACHIEVEMENT_ORDER);
    if(count == 1)
      needchange = true;

    const SItemCFG* pCur = ItemManager::getMe().getItemCFG(curAhieveTitle);
    if(pCur != nullptr && pCur->dwPostId == titleId)
      needchange = true;
    if(needchange)
      curAhieveTitle = titleId;

    m_pUser->setCollectMark(ECOLLECTTYPE_ACHIEVEMENT);
    MsgManager::sendMsg(m_pUser->id, 2870, MsgParams(pBase->strNameZh));
  }

  sendTitleData();
  if(needchange)
  {
    ChangeTitle cmd;
    TitleData* pData = cmd.mutable_title_data();
    pData->set_title_type(type);
    pData->set_id(titleId);
    cmd.set_charid(m_pUser->id);
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToNine(send, len);
  }
  m_pUser->getAchieve().onTitle(titleId);
  m_pUser->getMenu().refreshNewMenu(EMENUCOND_TITLE);
  m_pUser->getServant().onAppearEvent(ETRIGGER_TITLE);
  m_pUser->getServant().onFinishEvent(ETRIGGER_TITLE);

  for (auto &v : pBase->vecTitleBase)
  {
    const RoleData* pData = RoleDataConfig::getMe().getRoleData(v.type());
    if (pData != nullptr)
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_SHOWATTR, MsgParams(pData, v.value()));
  }

  XLOG << "[称号-添加] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << " id : " << titleId << XEND;
  
  if (titleId == 1002)    //勘察员
  {
    MailManager::getMe().sendMail(m_pUser->id, 10007);
  }
  else if (titleId == 1003) //F级冒险家
  {
    if (m_pUser->getUserSceneData().addFirstActionDone(EFIRSTACTION_FOOD_MAIL, false))
    {
      MailManager::getMe().sendMail(m_pUser->id, 10008);
    }
  }

  //platlog
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_Manual_Title;
  PlatLogManager::getMe().eventLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eid,
    m_pUser->getUserSceneData().getCharge(), eType, 0, 1);
  PlatLogManager::getMe().ManualLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eType,
    eid,
    EManual_Title,
    0,
    0,
    titleId);

  ItemInfo oInfo;
  oInfo.set_id(titleId);
  m_pUser->getEvent().onItemAdd(oInfo);
  return true;
}

bool Title::hasTitle(DWORD titleId)
{
  for (auto &m : m_mapTitle)
  {
    auto v = find(m.second.begin(), m.second.end(), titleId);
    if (v != m.second.end())
      return true;
  }

  return false;
}

DWORD Title::getCurTitle(ETitleType type)
{
  DWORD curTitle = 0;
  if(type == ETITLE_TYPE_MANNUAL)
  {
    auto it = m_mapTitle.find(type);
    if (it == m_mapTitle.end())
      return 0;

    curTitle = it->second[it->second.size() - 1];
  }
  else if(type == ETITLE_TYPE_ACHIEVEMENT || type == ETITLE_TYPE_ACHIEVEMENT_ORDER)
    curTitle = curAhieveTitle;

  return curTitle;
}

bool Title::canAdd(ETitleType type, DWORD titleId)
{
  if(hasTitle(titleId) == true)
  {
    XERR << "[称号-添加] 失败，已经获得称号 " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << " titleid:" << titleId << XEND;
    return false;
  }

  const SItemCFG* pBase = ItemManager::getMe().getItemCFG(titleId);
  if (pBase == nullptr)
  {
    XERR << "[称号-添加] 配置表错误 " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << " titleid:" << titleId << XEND;
    return false;
  }

  DWORD preID = ItemConfig::getMe().getTitlePreID(titleId); 
  if(preID != 0)
  {
    if(hasTitle(preID) == true)
      return true;
    else
      return false;
  }

  return true;
}

void Title::reloadConfigCheck()
{
  bool bChange = false;
  for (auto it = m_mapTitle.begin(); it != m_mapTitle.end();)
  {
    for (auto subIt = it->second.begin(); subIt != it->second.end();)
    {
      const SItemCFG* pBase = ItemManager::getMe().getItemCFG(*subIt);
      if(pBase == nullptr)
      {
        it->second.erase(subIt);
        bChange = true;
        XERR << "[称号-配置] 配置表中没找到 " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << " title:" << *subIt<< XEND;
      }
      else
      {
        ++subIt;
      }
    }

    if(it->second.empty())
      m_mapTitle.erase(it++);
    else
      ++it;
  }

  if(bChange == true)
    sendTitleData();
}

bool Title::changeCurTitle(ChangeTitle& data)
{
  ETitleType type = data.mutable_title_data()->title_type();
  if(type == ETITLE_TYPE_MANNUAL)
    return false;

  DWORD titleId = data.mutable_title_data()->id();
  if(titleId != 0)
  {
    const SItemCFG* pBase = ItemManager::getMe().getItemCFG(titleId);
    if (pBase == nullptr)
    {
      XERR << "[称号-切换] 配置表错误 " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << titleId <<  XEND;
      return false;
    }

    if(hasTitle(titleId) == false || titleId == curAhieveTitle)
      return false;

    DWORD dwPostId = pBase->dwPostId;
    if(dwPostId != 0)
    {
      if(hasTitle(dwPostId))
      {
        XERR << "[称号-切换] 失败,需要顺序切换 " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << " type:" << type << " titleid:" << titleId << ", want change titleid:" << dwPostId << XEND;
        return false;
      }
    }
  }

  DWORD curTitle = curAhieveTitle;
  curAhieveTitle = titleId;

  ChangeTitle cmd;
  TitleData* pData = cmd.mutable_title_data();
  pData->set_title_type(type);
  pData->set_id(titleId);
  cmd.set_charid(m_pUser->id);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToNine(send, len);

  XLOG << "[称号-切换] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "from id : " << curTitle << " to "<< titleId << XEND;
  return true;
}

//void Title::collectAttr(TVecAttrSvrs& attrs)
void Title::collectAttr()
{
  Attribute* pAttr = m_pUser->getAttribute();
  if (pAttr == nullptr)
    return;
  TSetDWORD setIDs;
  for (auto it = m_mapTitle.begin(); it != m_mapTitle.end(); ++it)
  {
    for (auto subIt = it->second.begin(); subIt != it->second.end(); ++subIt)
    {
      const SItemCFG* pBase = ItemManager::getMe().getItemCFG(*subIt);
      if(!pBase || pBase->vecTitleBase.empty())
      {
        continue;
      }
      for(auto &attr : pBase->vecTitleBase)
      {
        XDBG << "[称号-属性] 配置" << attr.ShortDebugString() << XEND;
        pAttr->modifyCollect(ECOLLECTTYPE_ACHIEVEMENT, attr);
      }
      /*{
        float value = attrs[attr.type()].value() + attr.value();
        attrs[attr.type()].set_value(value);
      }*/
      setIDs.insert(*subIt);
    }
  }
  XLOG << "[称号-属性] 添加" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "TitleID:" << setIDs << XEND;
}

void Title::sendCurrentTitle(ETitleType type)
{
  DWORD titleid = getCurTitle(type);
  ChangeTitle usercmd;
  TitleData* pCurData = usercmd.mutable_title_data();
  pCurData->set_title_type(type);
  pCurData->set_id(titleid);
  usercmd.set_charid(m_pUser->id);
  PROTOBUF(usercmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

DWORD Title::getTitleNum(ETitleType type)
{
  auto it = m_mapTitle.find(type);
  if (it == m_mapTitle.end())
    return 0;
  if (it->second.empty())
    return 0;

  return it->second.size();
}

bool Title::delTitle(ETitleType type, DWORD titleId)
{
  if(type == ETITLE_TYPE_MANNUAL)
  {
    auto vecTitle = m_mapTitle.find(type);
    if(vecTitle == m_mapTitle.end())
      return false;
    for(auto s = vecTitle->second.begin(); s != vecTitle->second.end(); s++)
    {
      if(*s == titleId)
      {
        vecTitle->second.erase(s, vecTitle->second.end());
        sendTitleData();
        return true;
      }
    }
  }

  return false;
}
