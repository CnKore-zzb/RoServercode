#include "Servant.h"
#include "SceneUser.h"
#include "SceneNpcManager.h"
#include "MsgManager.h"
#include "RewardConfig.h"
#include "ActivityManager.h"
#include "GuildCityManager.h"
#include "Menu.h"
#include "PetWork.h"

Servant::Servant(SceneUser* pUser) : m_pUser(pUser)
{

}

Servant::~Servant()
{

}

bool Servant::load(const BlobServant& oAccServant, const BlobServant& oCharServant)
{
  m_dwServantID = oCharServant.servantid();

  m_mapRecommendItems.clear();
  for( int i = 0; i < oAccServant.recitem_size(); ++i)
  {
    Cmd::RecommendItemInfo rRecommend = oAccServant.recitem(i);
    const SServantItemCFG* pItemCFG = ServantConfig::getMe().getServantCFG(rRecommend.dwid());
    if(pItemCFG == nullptr)
    {
      XERR << "[仆人-推荐加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id:" << rRecommend.dwid() << "配置不存在" << XEND;
      continue;
    }

    m_mapRecommendItems.emplace(rRecommend.dwid(), rRecommend);
  }
  for( int i = 0; i < oCharServant.recitem_size(); ++i)
  {
    Cmd::RecommendItemInfo rRecommend = oCharServant.recitem(i);
    const SServantItemCFG* pItemCFG = ServantConfig::getMe().getServantCFG(rRecommend.dwid());
    if(pItemCFG == nullptr)
    {
      XERR << "[仆人-推荐加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id:" << rRecommend.dwid() << "配置不存在" << XEND;
      continue;
    }

    m_mapRecommendItems.emplace(rRecommend.dwid(), rRecommend);
  }
  m_setOwnServant.clear();
  for (int i = 0; i < oCharServant.ownservant_size(); ++i)
    m_setOwnServant.emplace(oCharServant.ownservant(i));

  m_mapCurGrowthGroup.clear();
  for( int i = 0; i < oAccServant.growthcurinfo_size(); ++i)
  {
    Cmd::GrowthCurInfo gGrowth = oAccServant.growthcurinfo(i);
    m_mapCurGrowthGroup.emplace(gGrowth.type(), gGrowth.groupid());
  }
  m_mapGrowthItems.clear();
  for( int i = 0; i < oAccServant.growthitem_size(); ++i)
  {
    Cmd::GrowthItemInfo gGrowth = oAccServant.growthitem(i);
    const SGrowthItemCFG* pItemCFG = ServantConfig::getMe().getGrowthCFG(gGrowth.dwid());
    if(pItemCFG == nullptr)
    {
      XERR << "[仆人-提升加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id:" << gGrowth.dwid() << "配置不存在" << XEND;
      continue;
    }

    auto it = m_mapCurGrowthGroup.find(static_cast<EGrowthType>(pItemCFG->dwType));
    if(it != m_mapCurGrowthGroup.end() && pItemCFG->dwGroupID <= it->second)
      resetGrowthData(&gGrowth);
    m_mapGrowthItems.emplace(gGrowth.dwid(), gGrowth);
  }
  m_mapGrowthValues.clear();
  for( int i = 0; i < oAccServant.growthvalue_size(); ++i)
  {
    Cmd::GrowthValueInfo gGrowth = oAccServant.growthvalue(i);
    m_mapGrowthValues.emplace(gGrowth.groupid(), gGrowth);
  }

  if(m_dwServantID != 0)
  {
    //添加新配置内容
    const TMapServantCFG mapServantCFG = ServantConfig::getMe().getAllServantCFG();
    for(auto m = mapServantCFG.begin(); m != mapServantCFG.end(); ++m)
    {
      auto it = m_mapRecommendItems.find(m->first);
      if(it != m_mapRecommendItems.end())
        continue;

      RecommendItemInfo sItem;
      sItem.set_dwid(m->first);
      resetRecommendData(&sItem);
      m_mapRecommendItems.emplace(m->first, sItem);
    }

    if(m_mapGrowthItems.empty() == true)
      initDefaultGrowthData();
    else
    {
      const TMapGrowthCFG mapGrowthCFG = ServantConfig::getMe().getAllGrowthCFG();
      for(auto m = mapGrowthCFG.begin(); m != mapGrowthCFG.end(); ++m)
      {
        auto it = m_mapGrowthItems.find(m->first);
        if(it != m_mapGrowthItems.end())
          continue;

        GrowthItemInfo sItem;
        sItem.set_dwid(m->first);
        sItem.set_status(EGROWTH_STATUS_MIN);
        sItem.set_finishtimes(0);
        auto iter = m_mapCurGrowthGroup.find(static_cast<EGrowthType>(m->second.dwType));
        if(iter != m_mapCurGrowthGroup.end() && m->second.dwGroupID <= iter->second)
          resetGrowthData(&sItem);
        m_mapGrowthItems.emplace(m->first, sItem);
  }
    }
  }

  XLOG << "[仆人-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << " size: " << m_mapRecommendItems.size() << m_mapGrowthItems.size() << " servant num: "<< m_setOwnServant.size() << XEND;
  return true;
}

bool Servant::save(BlobServant* pAccBlob, BlobServant* pCharBlob)
{
  if (pAccBlob == nullptr || pCharBlob == nullptr)
  {
    XERR << "[仆人-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "保存失败,数据为空" << XEND;
    return false;
  }

  pAccBlob->Clear();
  pCharBlob->Clear();
  pCharBlob->set_servantid(m_dwServantID);

  for (auto m = m_mapRecommendItems.begin(); m != m_mapRecommendItems.end(); ++m)
  {
    const SServantItemCFG* pItemCFG = ServantConfig::getMe().getServantCFG(m->first);
    if(pItemCFG == nullptr)
      continue;
    Cmd::RecommendItemInfo *pItem = nullptr;
    if(pItemCFG->bIsAcc == false)
      pItem = pCharBlob->add_recitem();
    else
      pItem = pAccBlob->add_recitem();
    if(pItem == nullptr)
    {
      XERR << "[仆人-推荐保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id:" << m->first << "create protobuf error" << XEND;
      continue;
    }

    pItem->set_dwid(m->first);
    pItem->set_finishtimes(m->second.finishtimes());
    pItem->set_status(m->second.status());
  }
  for (auto m = m_mapGrowthItems.begin(); m != m_mapGrowthItems.end(); ++m)
  {
    const SGrowthItemCFG* pItemCFG = ServantConfig::getMe().getGrowthCFG(m->first);
    if(pItemCFG == nullptr)
      continue;
    Cmd::GrowthItemInfo *pItem = pAccBlob->add_growthitem();
    if(pItem == nullptr)
    {
      XERR << "[仆人-提升保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id:" << m->first << "create protobuf error" << XEND;
      continue;
    }

    pItem->set_dwid(m->first);
    pItem->set_finishtimes(m->second.finishtimes());
    pItem->set_status(m->second.status());
  }
  for (auto m = m_mapGrowthValues.begin(); m != m_mapGrowthValues.end(); ++m)
  {
    Cmd::GrowthValueInfo *pItem = pAccBlob->add_growthvalue();
    if(pItem == nullptr)
    {
      XERR << "[仆人-成长值保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id:" << m->first << "create protobuf error" << XEND;
      continue;
    }

    pItem->set_groupid(m->first);
    pItem->set_growth(m->second.growth());
    for(int i = 0; i < m->second.everreward_size(); ++i)
      pItem->add_everreward(m->second.everreward(i));
  }
  for (auto m = m_mapCurGrowthGroup.begin(); m != m_mapCurGrowthGroup.end(); ++m)
  {
    Cmd::GrowthCurInfo *pItem = pAccBlob->add_growthcurinfo();
    if(pItem == nullptr)
    {
      XERR << "[仆人-当前成长计划保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id:"
        << m->first << "create protobuf error" << XEND;
      continue;
    }

    pItem->set_type(m->first);
    pItem->set_groupid(m->second);
  }

  for (auto m = m_setOwnServant.begin(); m != m_setOwnServant.end(); ++m)
  {
    pCharBlob->add_ownservant(*m);
  }

  XDBG << "[仆人-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小 :" << pCharBlob->ByteSize() << "Acc: "
    << pAccBlob->ByteSize() << " size: " << m_mapRecommendItems.size() << m_mapGrowthItems.size()<< XEND;
  return true;
}

void Servant::initDefaultRecommendData()
{
  DWORD curTime = now();
  m_pUser->getVar().setVarValue(EVARTYPE_RECOMMEND_DAY, curTime);
  m_pUser->getVar().setVarValue(EVARTYPE_RECOMMEND_WEEK, curTime);

  const TMapServantCFG mapServantCFG = ServantConfig::getMe().getAllServantCFG();
  for(auto m = mapServantCFG.begin(); m != mapServantCFG.end(); ++m)
  {
    auto it = m_mapRecommendItems.find(m->first);
    if(it != m_mapRecommendItems.end())   //acc data
      continue;
    RecommendItemInfo sItem;
    sItem.set_dwid(m->first);

    m_mapRecommendItems.emplace(m->first, sItem);
  }

  for(auto it = m_mapRecommendItems.begin(); it != m_mapRecommendItems.end(); ++it)
  {
    if(checkAppearCondition(it->first) == true)
      setRecommendStatus(&(it->second), ERECOMMEND_STATUS_GO);
    if(it->second.status() == ERECOMMEND_STATUS_GO && checkFinishCondition(it->first, &(it->second)) == true)
      setRecommendStatus(&(it->second), ERECOMMEND_STATUS_RECEIVE);
  }

  sendAllRecommendInfo();
}

void Servant::resetRecommendData(RecommendItemInfo* pItem)
{
  if(pItem == nullptr)
    return;

  const SServantItemCFG* pItemCFG = ServantConfig::getMe().getServantCFG(pItem->dwid());
  if(pItemCFG == nullptr)
    return;

  DWORD dwOldFinish = pItem->finishtimes();
  if(pItem->status() == ERECOMMEND_STATUS_MIN && checkAppearCondition(pItem->dwid()) == true)
    setRecommendStatus(pItem, ERECOMMEND_STATUS_GO);
  if(pItem->status() == ERECOMMEND_STATUS_GO && checkFinishCondition(pItem->dwid(), pItem) == true)
    setRecommendStatus(pItem, ERECOMMEND_STATUS_RECEIVE);
  if(pItem->finishtimes() != dwOldFinish)
    refreshRcommendInfo(pItem->dwid());
}

void Servant::resetRecommendByType(ECycleType eType)
{
  for (auto it = m_mapRecommendItems.begin(); it != m_mapRecommendItems.end(); ++it)
  {
    const SServantItemCFG* pItemCFG = ServantConfig::getMe().getServantCFG(it->first);
    if(pItemCFG == nullptr || pItemCFG->eType != eType)
      continue;

    it->second.set_status(ERECOMMEND_STATUS_MIN);
    it->second.set_finishtimes(0);
    resetRecommendData(&(it->second));
  }

  bool bCanGetReward = canGetReward();
  if(bCanGetReward == false)
    m_pUser->getTip().removeRedTip(EREDSYS_SERVANT_RECOMMNED);

  m_pUser->setDataMark(EUSERDATATYPE_FAVORABILITY);
  m_pUser->refreshDataAtonce();
  sendFavorabilityStatus();
  XDBG << "[仆人-状态更新] : 类型更新 " << m_pUser->id << m_pUser->name << "类型: " << eType << XEND;
}

bool Servant::setRecommendStatus(RecommendItemInfo* pItem, ERecommendStatus eStatus, bool bCheck)
{
  if(pItem == nullptr)
    return false;
  const SServantItemCFG* pItemCFG = ServantConfig::getMe().getServantCFG(pItem->dwid());
  if(pItemCFG == nullptr)
    return false;

  if(pItem->status() + 1 == eStatus || bCheck == false)
  {
    pItem->set_status(eStatus);
    if(eStatus == ERECOMMEND_STATUS_RECEIVE)
      m_pUser->getTip().addRedTip(EREDSYS_SERVANT_RECOMMNED);
    XDBG << "[仆人-推荐状态]" << m_pUser->id << m_pUser->name << "推荐ID: " << pItem->dwid() << "状态: " << eStatus << XEND;
    return true;
  }

  return false;
}

void Servant::refreshRcommendInfo(DWORD dwid)
{
  RecommendServantUserCmd cmd;
  RecommendItemInfo *pItem = cmd.add_items();
  RecommendItemInfo *pRecItem = getRecommendItem(dwid);
  if(pItem == nullptr || pRecItem == nullptr || pRecItem->status() == ERECOMMEND_STATUS_MIN)
    return;

  pItem->set_dwid(dwid);
  pItem->set_finishtimes(pRecItem->finishtimes());
  pItem->set_status(pRecItem->status());
  pItem->set_realopen(pRecItem->realopen());

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

RecommendItemInfo* Servant::getRecommendItem(DWORD dwid)
{
  auto m = m_mapRecommendItems.find(dwid);
  if(m != m_mapRecommendItems.end())
    return &(m->second);

  return nullptr;
}

ERecommendStatus Servant::getRecommendStatus(DWORD dwID)
{
  auto m = m_mapRecommendItems.find(dwID);
  if(m != m_mapRecommendItems.end())
    return m->second.status();

  return ERECOMMEND_STATUS_MIN;
}

void Servant::timer(DWORD curTime)
{
  if(m_dwServantID == 0)
    return;

  if(checkReset() == true)
    sendAllRecommendInfo();
  if(m_dwCountDownStatus & ESTAY_STATUS_COUNT)
  {
    m_dwCountDown += 10;
    if(m_dwCountDown >= MiscConfig::getMe().getServantCFG().dwStayTime)
    {
      m_dwCountDownStatus = ESTAY_STATUS_COUNT_REWARD;
      sendStayFavoStatus(2);
    }
  }
  else if(m_dwCountDownStatus & ESTAY_STATUS_COUNT_REWARD)
  {
    m_dwCountDown += 10;
    if(m_dwCountDown >= MiscConfig::getMe().getServantCFG().dwDisappearTime)
    {
      m_dwCountDownStatus = ESTAY_STATUS_OVER;
      sendStayFavoStatus(3);
      m_dwCountDown = 0;
    }
  }
}

void Servant::processService(EServantService eType)
{
  switch (eType)
  {
    case ESERVANT_SERVICE_RECOMMEND:
      {
        sendAllRecommendInfo();
      }
      break;
    case ESERVANT_SERVICE_UPGRADE:
      {
      }
      break;
    case ESERVANT_SERVICE_SPECIAL:
      {
      }
      break;
    default:
      break;
  }
}

bool Servant::setServant(bool replace, DWORD servantid)
{
  if(m_pUser->getMenu().isOpen(EMENUID_SERVANT) == false)
    return false;
  if(MiscConfig::getMe().getServantCFG().isExist(servantid) == false)
    return false;
  const SNpcCFG* pNpcCfg = NpcConfig::getMe().getNpcCFG(servantid);
  if(pNpcCfg == nullptr)
    return false;

  if(replace == true)
  {
    if(m_dwServantID == 0)
      return false;
    if(servantid == m_dwServantID)
      return false;

    m_dwServantID = servantid;
    XLOG <<  "[仆人-更换], 成功" << m_pUser->name << m_pUser->id << "仆人ID:" << servantid << XEND;
  }
  else
  {
    if(m_dwServantID != 0)
      return false;

    m_dwServantID = servantid;
    initDefaultRecommendData();
    initDefaultGrowthData();
    XLOG <<  "[仆人-雇佣], 成功" << m_pUser->name << m_pUser->id << "仆人ID:" << servantid << XEND;
  }

  auto m = m_setOwnServant.find(servantid);
  if(m == m_setOwnServant.end())
  {
    m_setOwnServant.emplace(servantid);
    MsgManager::sendMsg(m_pUser->id, 25, MsgParams(pNpcCfg->strName));
  }

  m_pUser->setDataMark(EUSERDATATYPE_SERVANTID);
  m_pUser->refreshDataAtonce();
  return true;
}

void Servant::showServant(bool show)
{
  if(m_dwServantID == 0)
    return;

  if(show == true && m_qwServantNpcID == 0)
  {
    const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(m_dwServantID);
    if(pCFG == nullptr)
      return;

    if(m_pUser->getScene() == nullptr)
      return;

    float fRange = MiscConfig::getMe().getServantCFG().fRange;
    xPos pos = m_pUser->getPos();
    m_pUser->getScene()->getRandPos(m_pUser->getPos(), fRange, pos);
    DWORD dir = (m_pUser->getUserSceneData().getDir() / ONE_THOUSAND + 180) % 360;

    NpcDefine define;
    define.setID(pCFG->dwID);
    define.setPos(pos);
    define.setDir(dir);
    define.setBehaviours(define.getBehaviours() | BEHAVIOUR_MOVE_ABLE);
    define.m_oVar.m_qwOwnerID = m_pUser->id;
    stringstream sstr;
    sstr << m_pUser->name << MiscConfig::getMe().getServantCFG().getServantName(m_dwServantID);
    define.setName(sstr.str().c_str());

    SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(define, m_pUser->getScene());
    if (pNpc == nullptr)
      return;

    m_qwServantNpcID = pNpc->id;
    pNpc->setAttr(EATTRTYPE_MOVESPD, m_pUser->getMoveSpeed());
    xPos destPos = m_pUser->m_oMove.getDestPos();
    if(getXZDistance(pos, destPos) > fRange && m_pUser->m_oMove.empty() == false)
      onUserMove(destPos);

    if(canCountTime() == true)
    {
      m_dwCountDownStatus = ESTAY_STATUS_COUNT;
      sendStayFavoStatus(1);
  }
  }
  else if(show == false && m_qwServantNpcID != 0)
  {
    SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(m_qwServantNpcID);
    if (pNpc == nullptr)
      return;

    pNpc->removeAtonce();
    m_qwServantNpcID = 0;
    m_dwCountDownStatus = ESTAY_STATUS_MIN;
    m_dwCountDown = 0;
  }
}

void Servant::sendAllRecommendInfo()
{
  if(m_dwServantID == 0)
    return;

  checkReset();
  sendFavorabilityStatus();
  checkActivityRealOpen();

  RecommendServantUserCmd cmd;
  for(auto &it : m_mapRecommendItems)
  {
    if(it.second.status() == ERECOMMEND_STATUS_MIN)
      continue;
    RecommendItemInfo *pItem = cmd.add_items();
    pItem->set_dwid(it.first);
    pItem->set_finishtimes(it.second.finishtimes());
    pItem->set_status(it.second.status());
    pItem->set_realopen(it.second.realopen());
  }

  if(cmd.items_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
    XDBG <<  "[仆人-消息], 玩家" << m_pUser->name << m_pUser->id << "size: " << cmd.items_size() << "消息内容: " << cmd.ShortDebugString() << XEND;
  }
}

void Servant::getServantReward(bool bFavorability, DWORD dwID)
{
  if(m_dwServantID == 0)
    return;

  DWORD dwFavorability = m_pUser->getVar().getAccVarValue(EACCVARTYPE_FAVORABILITY);
  if(bFavorability == true)
  {
    if(dwID == 0)
    {
    std::vector<pair<DWORD, DWORD>> vecFavoReward = MiscConfig::getMe().getServantCFG().getFavorabilityReward();
    for(DWORD i = 0; i < vecFavoReward.size(); ++i)
    {
      if(dwFavorability < vecFavoReward[i].first)
        return;
      DWORD dwVarValue = MiscConfig::getMe().getServantCFG().getFavorabilityVar(vecFavoReward[i].first);
      DWORD dwFavoStatus = m_pUser->getVar().getAccVarValue(EACCVARTYPE_FAVORABILITY_STATUS);
      bool bGetReward = dwFavoStatus & dwVarValue;
      if(bGetReward == true)
        continue;
      DWORD dwRewardID = MiscConfig::getMe().getServantCFG().getReward(vecFavoReward[i].first);
      if(dwRewardID == 0)
        continue;
      m_pUser->getVar().setAccVarValue(EACCVARTYPE_FAVORABILITY_STATUS, dwFavoStatus+dwVarValue);
      m_pUser->getPackage().rollReward(dwRewardID, EPACKMETHOD_AVAILABLE, false, true);
      sendFavorabilityStatus();
        if(canCountTime())
        {
          m_dwCountDownStatus = ESTAY_STATUS_COUNT;
          sendStayFavoStatus(1);
        }

      XLOG <<  "[仆人-好感度奖励], 玩家" << m_pUser->name << m_pUser->id << "好感度数值:" << vecFavoReward[i].first << "获得奖励ID:" << dwRewardID << "status:" <<
        dwFavoStatus+dwVarValue << XEND;
      return;
    }
  }
  else
  {
      if((m_dwCountDownStatus & ESTAY_STATUS_COUNT_REWARD) == false)
        return;

      m_dwCountDownStatus = ESTAY_STATUS_OVER;
      sendStayFavoStatus(3);

      DWORD dwAddFavo = MiscConfig::getMe().getServantCFG().dwAddFavo;
      addFavoribility(dwAddFavo);
      XLOG <<  "[仆人-停留好感度奖励], 玩家" << m_pUser->name << m_pUser->id << "好感度数值:" << dwAddFavo << XEND;
      return;
    }
  }
  else
  {
    const SServantItemCFG* pItemCFG = ServantConfig::getMe().getServantCFG(dwID);
    if(pItemCFG == nullptr)
      return;

    RecommendItemInfo* pItem = getRecommendItem(dwID);
    if(pItem == nullptr || pItem->status() != ERECOMMEND_STATUS_RECEIVE)
      return;

    setRecommendStatus(pItem, ERECOMMEND_STATUS_FINISH);
    refreshRcommendInfo(dwID);
    if(pItemCFG->dwFavorability != 0)
    {
      addFavoribility(pItemCFG->dwFavorability);
    }

    if(pItemCFG->dwReward != 0)
    {
      m_pUser->getPackage().rollReward(pItemCFG->dwReward, EPACKMETHOD_AVAILABLE, false, true);
    }
    onAppearEvent(ETRIGGER_RECOMMEND);
    bool bCanGetReward = canGetReward();
    if(bCanGetReward == false)
      m_pUser->getTip().removeRedTip(EREDSYS_SERVANT_RECOMMNED);
    XLOG <<  "[仆人-今日推荐], 领奖" << m_pUser->name << m_pUser->id << "推荐ID:" << dwID << "Favorability: " << pItemCFG->dwFavorability
      << "rewardID: " << pItemCFG->dwReward << XEND;
  }
}

bool Servant::addFavoribility(DWORD dwValue)
{
  if(m_dwServantID == 0)
    return false;

  DWORD dwFavorability = m_pUser->getVar().getAccVarValue(EACCVARTYPE_FAVORABILITY);
  DWORD dwMaxFavorability = MiscConfig::getMe().getServantCFG().dwMaxFavorability;
  if(dwFavorability >= dwMaxFavorability)
    return false;

  DWORD dwTotal = (dwFavorability + dwValue) > dwMaxFavorability ? dwMaxFavorability : (dwFavorability + dwValue);
  m_pUser->getVar().setAccVarValue(EACCVARTYPE_FAVORABILITY, dwTotal);
  m_pUser->setDataMark(EUSERDATATYPE_FAVORABILITY);
  m_pUser->refreshDataAtonce();
  sendFavorabilityStatus();

  XLOG << "[仆人-好感度]" << m_pUser->accid << m_pUser->id << m_pUser->name << " 好感度: " << dwFavorability << " after: " << dwTotal << XEND;
  return true;
}

void Servant::sendFavorabilityStatus()
{
  DWORD dwFavo = m_pUser->getVar().getAccVarValue(EACCVARTYPE_FAVORABILITY);
  DWORD dwStatus = m_pUser->getVar().getAccVarValue(EACCVARTYPE_FAVORABILITY_STATUS);

  Cmd::ServantRewardStatusUserCmd cmd;
  std::vector<pair<DWORD, DWORD>> vecFavoReward = MiscConfig::getMe().getServantCFG().getFavorabilityReward();
  for(DWORD i = 0; i < vecFavoReward.size(); ++i)
  {
    Cmd::FavorabilityStatus* pItem = cmd.add_items();
    if(pItem == nullptr)
      continue;
    pItem->set_favorability(vecFavoReward[i].first);
    if(dwFavo >= vecFavoReward[i].first)
    {
      DWORD dwVarValue = MiscConfig::getMe().getServantCFG().getFavorabilityVar(vecFavoReward[i].first);
      bool bGetReward = dwStatus & dwVarValue;
      if(bGetReward == true)
        pItem->set_status(2);
      else
        pItem->set_status(1);
    }
  }

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

bool Servant::checkAppearCondition(DWORD dwid)
{
  if(m_dwServantID == 0)
    return false;

  const SServantItemCFG* pItemCFG = ServantConfig::getMe().getServantCFG(dwid);
  if(pItemCFG == nullptr)
    return false;

  DWORD dwParam = 0;
  if(pItemCFG->stAppearCondition.vecParams.empty() == false)
    dwParam = pItemCFG->stAppearCondition.vecParams[0];

  switch (pItemCFG->stAppearCondition.eTrigger)
  {
    case ETRIGGER_MIN:
      {
        return true;
      }
      break;
    case ETRIGGER_TIME_INTERVAL:
      {
        if(pItemCFG->stAppearCondition.vecParams.size() < 4)
          return false;
        DWORD curTime = xTime::getCurSec();
        return ServantConfig::getMe().checkInTimeInterval(curTime, pItemCFG->stAppearCondition.vecParams[0],
            pItemCFG->stAppearCondition.vecParams[1], pItemCFG->stAppearCondition.vecParams[2], pItemCFG->stAppearCondition.vecParams[3]);
      }
    case ETRIGGER_ITEM_GET:
      {
        DWORD dwCount = 1;
        if(pItemCFG->stAppearCondition.vecParams.size() > 1)
          dwCount = pItemCFG->stAppearCondition.vecParams[1];

        return getItemCount(dwParam) >= dwCount;
      }
      break;
    case ETRIGGER_QUEST_SUBMIT:
      {
        return checkQuest(dwid, true, nullptr);
      }
      break;
    case ETRIGGER_LEVELUP:
      {
        return m_pUser->getLevel() >= dwParam;
      }
      break;
    case ETRIGGER_JOBLEVELUP:
      {
        return m_pUser->getJobLv() >= dwParam;
      }
      break;
    case ETRIGGER_MENU:
      {
        return checkMenu(dwid);
      }
      break;
    case ETRIGGER_OWN_STUDENT:
      {
        return m_pUser->getSocial().getRelationCount(ESOCIALRELATION_STUDENT) >= dwParam;
      }
      break;
    case ETRIGGER_JOIN_GUILD:
      {
        return m_pUser->hasGuild();
      }
      break;
    case ETRIGGER_TITLE:
      {
        return m_pUser->getTitle().hasTitle(dwParam);
      }
      break;
    case ETRIGGER_RECOMMEND:
      {
        return getRecommendStatus(dwParam) == ERECOMMEND_STATUS_FINISH;
      }
      break;
    case ETRIGGER_MAPID:
      {
        return m_pUser->getUserSceneData().isNewMap(dwParam) == false;
      }
      break;
    case ETRIGGER_GUILD_BUILDING:
      {
        GuildScene* pScene = dynamic_cast<GuildScene*>(m_pUser->getScene());
        if(pScene == nullptr)
          return false;
        DWORD dwLv = 1;
        if(pItemCFG->stAppearCondition.vecParams.size() > 1)
          dwLv = pItemCFG->stAppearCondition.vecParams[1];
        return pScene->getGuild().getBuildingNum(dwLv) >= dwParam;
      }
      break;
    default:
      return false;
  }

  return false;
}

bool Servant::checkFinishCondition(DWORD dwid, RecommendItemInfo* pItem)
{
  if(m_dwServantID == 0 || pItem == nullptr)
    return false;

  const SServantItemCFG* pItemCFG = ServantConfig::getMe().getServantCFG(dwid);
  if(pItemCFG == nullptr)
    return false;

  DWORD dwParam = 0;
  if(pItemCFG->stFinishCondition.vecParams.empty() == false)
    dwParam = pItemCFG->stFinishCondition.vecParams[0];

  switch (pItemCFG->stFinishCondition.eTrigger)
  {
    case ETRIGGER_MIN:
      {
        return true;
      }
      break;
    case ETRIGGER_CAPRA:
      {
        return false;     //暂时不做
      }
      break;
    case ETRIGGER_TOWER_PASS:
      {
        m_pUser->getTower().resetData();
        if(m_pUser->getTower().getCurMaxLayer() >= dwParam)
        {
          pItem->set_finishtimes(dwParam);
          return true;
        }
        return false;
      }
      break;
    case ETRIGGER_QUEST_SUBMIT:
      {
        return checkQuest(dwid, false, pItem);
      }
      break;
    case ETRIGGER_ITEM_GET:
      {
        DWORD dwCount = 1;
        if(pItemCFG->stFinishCondition.vecParams.size() > 1)
          dwCount = pItemCFG->stFinishCondition.vecParams[1];

        return getItemCount(dwParam) >= dwCount;
      }
      break;
    case ETRIGGER_OWN_TUTOR:
      {
        bool bAchieve = m_pUser->getAchieve().isFinishAchieve(1206009);
        return m_pUser->getSocial().getRelationCount(ESOCIALRELATION_TUTOR) >= dwParam || bAchieve;
      }
      break;
    case ETRIGGER_TITLE:
      {
        return m_pUser->getTitle().hasTitle(dwParam);
      }
      break;
    case ETRIGGER_MENU:
      {
        return checkMenu(dwid);
      }
      break;
    case ETRIGGER_JOIN_GUILD:
      {
        return m_pUser->hasGuild();
      }
      break;
    case ETRIGGER_CHANGE_PROFESSION:
      {
        return static_cast<DWORD>(m_pUser->getProfession() % 10) >= dwParam || m_pUser->m_oProfession.getProfessionMax() % 10 >= dwParam;
      }
      break;
    case ETRIGGER_MERCENARY_CAT:
      {
        return m_pUser->getWeaponPet().has();
      }
      break;
    case ETRIGGER_PHOTO_SCENERY:
      {
        if(m_pUser->getManual().getNumByStatus(EMANUALTYPE_SCENERY, EMANUALSTATUS_UNLOCK) >= dwParam)
        {
          pItem->set_finishtimes(dwParam);
          return true;
        }
        pItem->set_finishtimes(m_pUser->getManual().getNumByStatus(EMANUALTYPE_SCENERY, EMANUALSTATUS_UNLOCK));
      }
      break;
    case ETRIGGER_BATTLE_TIME:
      {
        m_pUser->antiAddictRefresh();
        DWORD dwRealTime = m_pUser->getBattleTime() + m_pUser->getUserSceneData().getUsedBattleTime();
        if(dwRealTime >= dwParam)
        {
          pItem->set_finishtimes(dwParam/60);
          return true;
        }
        pItem->set_finishtimes(dwRealTime/60);
      }
      break;
    case ETRIGGER_PRODUCE_HEAD:
      {
        if(m_pUser->getManual().getNumByStatus(EMANUALTYPE_FASHION, EMANUALSTATUS_UNLOCK) >= dwParam)
        {
          pItem->set_finishtimes(dwParam);
          return true;
        }
        pItem->set_finishtimes(m_pUser->getManual().getNumByStatus(EMANUALTYPE_FASHION, EMANUALSTATUS_UNLOCK));
      }
      break;
    case ETRIGGER_GUILD_PRAY:
      {
        return m_pUser->getGuildPrayTimes() >= dwParam;
      }
      break;
    case ETRIGGER_MAXJOBLEVEL:
      {
        if(m_pUser->getCurFighter() != nullptr)
          return m_pUser->getCurFighter()->getMaxJobLv() >= dwParam;
      }
      break;
    case ETRIGGER_UNLOCK_CATNUM:
      {
        return m_pUser->getWeaponPet().getMaxSize() >= dwParam;
      }
      break;
    case ETRIGGER_POLLY_FIGHT:
    case ETRIGGER_CAT_INVASION:
    case ETRIGGER_GVG:
    case ETRIGGER_GUILD_FUBEN:
    case ETRIGGER_PET_ADVENTURE:
    case ETRIGGER_PETWORK:
    case ETRIGGER_TUTOR_SKILL:
    case ETRIGGER_AUGURY:
    case ETRIGGER_WANTED_QUEST_DAY:
    case ETRIGGER_DAILYMONSTER:
    case ETRIGGER_REPAIR_SEAL:
    case ETRIGGER_LABORATORY:
    case ETRIGGER_GUILD_DONATE:
    case ETRIGGER_LISTEN_MUSIC:
    case ETRIGGER_PLAY_MUSIC:
    case ETRIGGER_PVP_KILL:
    case ETRIGGER_EAT_FOOD:
    case ETRIGGER_DOJO:
    case ETRIGGER_ENCHANT_PRIMARY:
    case ETRIGGER_ENCHANT_MIDDLE:
    case ETRIGGER_ENCHANT_ADVANCED:
    case ETRIGGER_COOKFOOD:
    case ETRIGGER_HAIR:
    case ETRIGGER_EYE:
    case ETRIGGER_LOTTERY:
    case ETRIGGER_GUILD_BUILD_DONATE:
    case ETRIGGER_WORLD_FREYJA:
    case ETRIGGER_CARD_RESET:
    case ETRIGGER_CARD_CUSTOMIZE:
    case ETRIGGER_ENCHANT_HEAD:
    case ETRIGGER_KILL_MINI:
    case ETRIGGER_KILL_MVP:
    case ETRIGGER_PVE_CARD:
    case ETRIGGER_KILL_STAR_NPC:
    case ETRIGGER_MVP_BATTLE:
      {
        if(pItem->finishtimes() >= dwParam)
        {
          pItem->set_finishtimes(dwParam);
          return true;
        }
      }
      break;
    default:
      return false;
  }
  return false;
}

bool Servant::onAppearEvent(ETriggerType eType)
{
  if(m_dwServantID == 0)
    return false;

  checkReset();
  for(auto &it : m_mapRecommendItems)
  {
    if(ServantConfig::getMe().getRecoTrigger(it.first) != eType)
      continue;

    if(it.second.status() == ERECOMMEND_STATUS_MIN && checkAppearCondition(it.first) == true)
    {
      setRecommendStatus(&(it.second), ERECOMMEND_STATUS_GO);
      if(checkFinishCondition(it.first, &(it.second)) == true)
        setRecommendStatus(&(it.second), ERECOMMEND_STATUS_RECEIVE);
      refreshRcommendInfo(it.first);
    }
  }

  return false;
}

bool Servant::onFinishEvent(ETriggerType eType, DWORD dwParam)
{
  if(m_dwServantID == 0)
    return false;

  checkReset();

  for(auto &it : m_mapRecommendItems)
  {
    const SServantItemCFG* pItemCFG = ServantConfig::getMe().getServantCFG(it.first);
    if(pItemCFG == nullptr || pItemCFG->stFinishCondition.eTrigger != eType)
      continue;
    if(it.second.status() > ERECOMMEND_STATUS_GO)
      continue;
    if(it.second.status() == ERECOMMEND_STATUS_MIN && pItemCFG->stAppearCondition.eTrigger != ETRIGGER_RECOMMEND)
      continue;

    DWORD dwTimes = it.second.finishtimes();
    if(pItemCFG->stFinishCondition.eTrigger != ETRIGGER_QUEST_SUBMIT && pItemCFG->stFinishCondition.eTrigger != ETRIGGER_MENU)
      it.second.set_finishtimes(dwTimes + dwParam);
    if(checkFinishCondition(it.first, &(it.second)) == true)
      setRecommendStatus(&(it.second), ERECOMMEND_STATUS_RECEIVE);
    refreshRcommendInfo(it.first);
  }

  return false;
}

void Servant::onUserAttack(const SceneNpc* pNpc)
{
  if(m_dwServantID == 0)
    return;

  if(pNpc == nullptr)
    return;

  if(pNpc->getNpcID() == 30025)   //飞碟猫
    onFinishEvent(ETRIGGER_CAT_INVASION);
}

void Servant::onKillNpc(const SceneNpc* pNpc)
{
  if(m_dwServantID == 0)
    return;

  if(pNpc == nullptr)
    return;

  if(pNpc->getNpcID() == 120094 || pNpc->getNpcID() == 120114 || pNpc->getNpcID() == 120122)    //星怪
    onFinishEvent(ETRIGGER_KILL_STAR_NPC);
}

void Servant::onUserMove(const xPos& dest)
{
  if(m_dwServantID == 0 || m_qwServantNpcID == 0)
    return;
  Scene* pScene = m_pUser->getScene();
  if (pScene == nullptr)
    return;
  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(m_qwServantNpcID);
  if(!pNpc)
    return;

  std::list<xPos> list;
  if (pScene->findingPath(dest, m_pUser->getPos(), list, TOOLMODE_PATHFIND_FOLLOW) == false)
    return;
  if (list.size() < 2)
    return;

  list.pop_front();
  xPos dest1 = list.front();
  xPos mydest = dest1;
  float foldis = 1;
  float dist = getDistance(dest, dest1);
  mydest.x = foldis / dist * (dest1.x - dest.x) + dest.x;
  mydest.z = foldis / dist * (dest1.z - dest.z) + dest.z;

  auto getPosOnCircle = [&](const xPos& pos0, const xPos& pos1, float angle, xPos& out)
  {
    angle = angle * 3.14 / 180.0f;
    out = pos0;
    out.z = pos0.z + (pos1.z - pos0.z) * cos(angle) - (pos1.x - pos0.x) * sin(angle);
    out.x = pos0.x + (pos1.z - pos0.z) * sin(angle) + (pos1.x - pos0.x) * cos(angle);
  };
  xPos outpos;
  getPosOnCircle(dest, mydest, -70, outpos);
  if (pScene->getValidPos(outpos) == false)
    return;
  pNpc->m_ai.moveTo(outpos);
}

void Servant::onUserMoveTo(const xPos& pos)
{
  //if(m_dwServantID == 0 || m_qwServantNpcID == 0)
  //  return;

  //SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(m_qwServantNpcID);
  //if(pNpc)
  //  pNpc->setScenePos(pos);
}

void Servant::onUserGoTo(const xPos& dest)
{
  if(m_dwServantID == 0 || m_qwServantNpcID == 0)
    return;

  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(m_qwServantNpcID);
  if(pNpc && pNpc->getScene())
  {
    xPos pos = dest;
    pNpc->getScene()->getRandPos(m_pUser->getPos(), 5, pos);
    pNpc->m_oMove.clear();
    pNpc->goTo(pos);
  }
}

bool Servant::checkMenu(DWORD dwid)
{
  const SServantItemCFG* pItemCFG = ServantConfig::getMe().getServantCFG(dwid);
  if(pItemCFG == nullptr)
    return false;
  if(pItemCFG->stAppearCondition.eTrigger != ETRIGGER_MENU || pItemCFG->stAppearCondition.vecParams.empty() == true)
    return false;

  if(pItemCFG->stAppearCondition.vecParams.size() == 1)
    return m_pUser->getMenu().isOpen(pItemCFG->stAppearCondition.vecParams[0]);
  else if(pItemCFG->stAppearCondition.vecParams.size() > 1)
  {
    if(pItemCFG->stAppearCondition.vecParams[0] == 0)    //或
    {
      for(auto it = pItemCFG->stAppearCondition.vecParams.begin(); it != pItemCFG->stAppearCondition.vecParams.end(); ++it)
      {
        if(it == pItemCFG->stAppearCondition.vecParams.begin())
          continue;
        if(m_pUser->getMenu().isOpen(*it) == true)
          return true;
      }
      return false;
    }
    else    //与
    {
      for(auto it = pItemCFG->stAppearCondition.vecParams.begin(); it != pItemCFG->stAppearCondition.vecParams.end(); ++it)
      {
        if(it == pItemCFG->stAppearCondition.vecParams.begin())
          continue;
        if(m_pUser->getMenu().isOpen(*it) == false)
          return false;
      }
      return true;
    }
  }

  return false;
}

bool Servant::checkQuest(DWORD dwid, bool isAppearEvent, RecommendItemInfo* pItem)
{
  const SServantItemCFG* pItemCFG = ServantConfig::getMe().getServantCFG(dwid);
  if(pItemCFG == nullptr)
    return false;
  if(isAppearEvent == true)
  {
    if(pItemCFG->stAppearCondition.vecParams.empty() == true || pItemCFG->stAppearCondition.eTrigger != ETRIGGER_QUEST_SUBMIT)
      return false;
    if(pItemCFG->stAppearCondition.vecParams.size() == 1)
      return m_pUser->getQuest().isSubmit(pItemCFG->stAppearCondition.vecParams[0]);
    else if(pItemCFG->stAppearCondition.vecParams.size() > 1)
    {
      if(pItemCFG->stAppearCondition.vecParams[0] == 0)    //或
      {
        for(auto it = pItemCFG->stAppearCondition.vecParams.begin(); it != pItemCFG->stAppearCondition.vecParams.end(); ++it)
        {
          if(it == pItemCFG->stAppearCondition.vecParams.begin())
            continue;
          if(m_pUser->getQuest().isSubmit(*it) == true)
            return true;
        }
        return false;
      }
      else    //与
      {
        for(auto it = pItemCFG->stAppearCondition.vecParams.begin(); it != pItemCFG->stAppearCondition.vecParams.end(); ++it)
        {
          if(it == pItemCFG->stAppearCondition.vecParams.begin())
            continue;
          if(m_pUser->getQuest().isSubmit(*it) == false)
            return false;
        }
        return true;
      }
    }
  }
  else
  {
    if(pItemCFG->stFinishCondition.vecParams.empty() == true || pItemCFG->stFinishCondition.eTrigger != ETRIGGER_QUEST_SUBMIT)
      return false;
    if(pItemCFG->stFinishCondition.vecParams.size() == 1)
      return m_pUser->getQuest().isSubmit(pItemCFG->stFinishCondition.vecParams[0]);
    else if(pItemCFG->stFinishCondition.vecParams.size() > 1)
    {
      if(pItemCFG->stFinishCondition.vecParams[0] == 0)    //或
      {
        for(auto it = pItemCFG->stFinishCondition.vecParams.begin(); it != pItemCFG->stFinishCondition.vecParams.end(); ++it)
        {
          if(it == pItemCFG->stFinishCondition.vecParams.begin())
            continue;
          if(m_pUser->getQuest().isSubmit(*it) == true)
            return true;
        }
        return false;
      }
      else    //与
      {
        DWORD dwNum = 0;
        for(DWORD i = 1; i < pItemCFG->stFinishCondition.vecParams.size(); ++i)
        {
          if(m_pUser->getQuest().isSubmit(pItemCFG->stFinishCondition.vecParams[i]) == true)
            dwNum++;
        }
        if(pItem != nullptr && dwNum > pItem->finishtimes())
        {
          pItem->set_finishtimes(dwNum);
          refreshRcommendInfo(dwid);
        }
        if(dwNum >= pItemCFG->stFinishCondition.vecParams.size() - 1)
          return true;
        return false;
      }
    }
  }

  return false;
}

bool Servant::checkReset()
{
  bool ret = false;
  if(m_pUser->getVar().getVarValue(EVARTYPE_RECOMMEND_DAY) == 0)
  {
    ret = true;
    resetRecommendByType(ECYCLE_DAY);
    m_pUser->getVar().setVarValue(EVARTYPE_RECOMMEND_DAY, now());
    XLOG << "[仆人-重置] 天 " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << XEND;
  }
  if(m_pUser->getVar().getVarValue(EVARTYPE_RECOMMEND_WEEK) == 0)
  {
    ret = true;
    resetRecommendByType(ECYCLE_WEEK);
    m_pUser->getVar().setVarValue(EVARTYPE_RECOMMEND_WEEK, now());
    XLOG << "[仆人-重置] 周 " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << XEND;
  }

  return ret;
}

void Servant::checkActivityRealOpen()
{
  for(auto &it : m_mapRecommendItems)
  {
    const SServantItemCFG* pItemCFG = ServantConfig::getMe().getServantCFG(it.first);
    if(pItemCFG == nullptr)
      continue;

    switch (pItemCFG->stFinishCondition.eTrigger)
    {
      case ETRIGGER_POLLY_FIGHT:
      //case ETRIGGER_CAPRA:
      case ETRIGGER_CAT_INVASION:
      case ETRIGGER_MVP_BATTLE:
        {
          DWORD dwParam = 0;
          if(ETRIGGER_POLLY_FIGHT == pItemCFG->stFinishCondition.eTrigger)
            dwParam = 205;
          else if(ETRIGGER_CAT_INVASION == pItemCFG->stFinishCondition.eTrigger)
            dwParam = 1;
          else if(ETRIGGER_MVP_BATTLE == pItemCFG->stFinishCondition.eTrigger)
            dwParam = 400;
          if(ActivityManager::getMe().isOpen(dwParam, false) == true && it.second.realopen() == false)
          {
            it.second.set_realopen(true);
            refreshRcommendInfo(it.first);
          }
          else if(ActivityManager::getMe().isOpen(dwParam, false) == false && it.second.realopen() == true)
          {
            it.second.set_realopen(false);
            refreshRcommendInfo(it.first);
          }
        }
        break;
      case ETRIGGER_GVG:
        {
          if(GuildCityManager::getMe().isInFire() == true && it.second.realopen() == false)
          {
            it.second.set_realopen(true);
            refreshRcommendInfo(it.first);
          }
          else if(GuildCityManager::getMe().isInFire() == false && it.second.realopen() == true)
          {
            it.second.set_realopen(false);
            refreshRcommendInfo(it.first);
          }
        }
        break;
      default:
        break;
    }
  }
}

bool Servant::canGetReward() const
{
  for(auto &it : m_mapRecommendItems)
  {
    if(it.second.status() == ERECOMMEND_STATUS_RECEIVE)
      return true;
  }

  return false;
}

void Servant::checkForeverRecommend()
{
  if(m_dwServantID == 0)
    return;

  for(auto &it : m_mapRecommendItems)
  {
    const SServantItemCFG* pItemCFG = ServantConfig::getMe().getServantCFG(it.first);
    if(pItemCFG == nullptr || (pItemCFG->eType != ECYCLE_FOREVER && pItemCFG->eType != ECYCLE_FOREVER_GUIDE))
      continue;

    resetRecommendData(&(it.second));
  }
}

bool Servant::canCountTime() const
{
  if(m_dwCountDownStatus != ESTAY_STATUS_MIN)
    return false;

  DWORD dwFavo = m_pUser->getVar().getAccVarValue(EACCVARTYPE_FAVORABILITY);
  DWORD dwStatus = m_pUser->getVar().getAccVarValue(EACCVARTYPE_FAVORABILITY_STATUS);

  const SServantCFG rCFG = MiscConfig::getMe().getServantCFG();
  std::vector<pair<DWORD, DWORD>> vecFavoReward = rCFG.getFavorabilityReward();
  for(DWORD i = 0; i < vecFavoReward.size(); ++i)
  {
    if(dwFavo >= vecFavoReward[i].first)
    {
      DWORD dwVarValue = rCFG.getFavorabilityVar(vecFavoReward[i].first);
      bool bGetReward = dwStatus & dwVarValue;
      if(bGetReward == false)
        return false;
    }
  }

  if(m_pUser->getVar().getAccVarValue(EACCVARTYPE_STAY_FAVORABILITY) >= rCFG.dwStayNum)
    return false;

  return true;
}

void Servant::sendStayFavoStatus(DWORD status) const
{
  ServantRewardStatusUserCmd cmd;
  cmd.set_stayfavo(status);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

//--------------女仆提升功能---------------//

void Servant::sendAllGrowthInfo()
{
  if(m_dwServantID == 0)
    return;

  GrowthServantUserCmd cmd;
  for(auto &m : m_mapCurGrowthGroup)
  {
    GrowthGroupInfo *pData = cmd.add_datas();
    for(auto &it : m_mapGrowthItems)
    {
      if(it.second.dwid() / SERVANT_GROWTH_ID_PARAM != m.second)
        continue;
      GrowthItemInfo *pItem = pData->add_items();
      if(pItem == nullptr)
        continue;
      pItem->set_dwid(it.first);
      pItem->set_finishtimes(it.second.finishtimes());
      pItem->set_status(it.second.status());
    }

    auto it = m_mapGrowthValues.find(m.second);
    if(it != m_mapGrowthValues.end())
    {
      Cmd::GrowthValueInfo *pItem = pData->mutable_valueitems();
      if(pItem == nullptr)
        continue;

      pItem->set_groupid(m.second);
      pItem->set_growth(it->second.growth());
      for(int i = 0; i < it->second.everreward_size(); ++i)
        pItem->add_everreward(it->second.everreward(i));
    }
  }

  for(auto &s : m_setUnlockedGrowth)
    cmd.add_unlockitems(s);

  if(cmd.datas_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
    XDBG <<  "[仆人-提升消息], 玩家" << m_pUser->name << m_pUser->id << "size: " << cmd.datas_size() << "消息内容: " << cmd.ShortDebugString() << XEND;
  }
}

void Servant::sendGrowthInfo(DWORD groupid)
{
  GrowthServantUserCmd cmd;
  GrowthGroupInfo *pData = cmd.add_datas();
  if(pData == nullptr)
    return;

  for(auto &it : m_mapGrowthItems)
  {
    if(it.second.dwid() / SERVANT_GROWTH_ID_PARAM != groupid)
      continue;

    resetGrowthData(&it.second);

    GrowthItemInfo *pItem = pData->add_items();
    pItem->set_dwid(it.first);
    pItem->set_finishtimes(it.second.finishtimes());
    pItem->set_status(it.second.status());
  }

  auto it = m_mapGrowthValues.find(groupid);
  if(it != m_mapGrowthValues.end())
  {
    Cmd::GrowthValueInfo *pItem = pData->mutable_valueitems();
    if(pItem == nullptr)
      return;

    pItem->set_groupid(groupid);
    pItem->set_growth(it->second.growth());
    for(int i = 0; i < it->second.everreward_size(); ++i)
      pItem->add_everreward(it->second.everreward(i));
  }

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

bool Servant::checkGrowthGroupAppearCond(DWORD dwid)
{
  const SGrowthItemCFG* pItemCFG = ServantConfig::getMe().getGrowthCFG(dwid);
  if(pItemCFG == nullptr)
    return false;

  DWORD dwParam = 0;
  if(pItemCFG->stGroupAppearCond1.vecParams.empty() == false)
    dwParam = pItemCFG->stGroupAppearCond1.vecParams[0];

  if(m_pUser->getLevel() < dwParam)
    return false;

  DWORD dwParam2 = 0;
  if(pItemCFG->stGroupAppearCond2.vecParams.empty() == false)
    dwParam2 = pItemCFG->stGroupAppearCond2.vecParams[0];

  if(dwParam2 != 0 && checkGrowthGroupFinishCond(dwParam2) == false)
    return false;

  return true;
}

bool Servant::checkGrowthGroupFinishCond(DWORD dwGroupID)
{
  for(auto &s : m_mapGrowthItems)
  {
    DWORD group = s.first / SERVANT_GROWTH_ID_PARAM;
    if(group != dwGroupID)
      continue;

    if(s.second.status() != EGROWTH_STATUS_FINISH)
      return false;
  }

  //check 成长值奖励是否全部领取
  DWORD dwCanGetRewardNum = MiscConfig::getMe().getServantCFG().getGrowthRewardNum(dwGroupID);
  DWORD dwGetRewardNum = m_mapGrowthValues[dwGroupID].everreward_size();
  if(dwCanGetRewardNum > dwGetRewardNum)
    return false;

  return true;
}

bool Servant::checkGrowthAppearCond(DWORD dwid)
{
  if(m_dwServantID == 0)
    return false;

  const SGrowthItemCFG* pItemCFG = ServantConfig::getMe().getGrowthCFG(dwid);
  if(pItemCFG == nullptr)
    return false;

  if(checkGrowthGroupAppearCond(dwid) == false)
    return false;

  DWORD dwParam = 0;
  if(pItemCFG->stGrowthAppearCond.vecParams.empty() == false)
    dwParam = pItemCFG->stGrowthAppearCond.vecParams[0];

  switch (pItemCFG->stGrowthAppearCond.eTrigger)
  {
    case ETRIGGER_MIN:
      {
        return true;
      }
      break;
    case ETRIGGER_ITEM_GET:
      {
        DWORD dwCount = 1;
        if(pItemCFG->stGrowthAppearCond.vecParams.size() > 1)
          dwCount = pItemCFG->stGrowthAppearCond.vecParams[1];

        return getItemCount(dwParam) >= dwCount;
      }
      break;
    case ETRIGGER_QUEST_SUBMIT:
      {
        return checkGrowthQuest(dwid, true, nullptr, ETRIGGER_QUEST_SUBMIT);
      }
      break;
    case ETRIGGER_LEVELUP:
      {
        return m_pUser->getLevel() >= dwParam;
      }
      break;
    case ETRIGGER_MENU:
      {
        return checkGrowthMenu(dwid, true);
      }
      break;
    case ETRIGGER_OWN_STUDENT:
      {
        return m_pUser->getSocial().getRelationCount(ESOCIALRELATION_STUDENT) >= dwParam;
      }
      break;
    case ETRIGGER_JOIN_GUILD:
      {
        return m_pUser->hasGuild();
      }
      break;
    case ETRIGGER_TITLE:
      {
        return m_pUser->getTitle().hasTitle(dwParam);
      }
      break;
    case ETRIGGER_MAPID:
      {
        return m_pUser->getUserSceneData().isNewMap(dwParam) == false;
      }
      break;
    case ETRIGGER_GUILD_BUILDING:
      {
        GuildScene* pScene = dynamic_cast<GuildScene*>(m_pUser->getScene());
        if(pScene == nullptr)
          return false;
        DWORD dwLv = 1;
        if(pItemCFG->stGrowthAppearCond.vecParams.size() > 1)
          dwLv = pItemCFG->stGrowthAppearCond.vecParams[1];
        return pScene->getGuild().getBuildingNum(dwLv) >= dwParam;
      }
      break;
      //女仆提升
    case ETRIGGER_LAST_GROWTH:
      {
        return getGrowthStatus(dwParam) == EGROWTH_STATUS_FINISH;
      }
      break;
    case ETRIGGER_CHANGE_PROFESSION:
      {
        return static_cast<DWORD>(m_pUser->getProfession() % 10) >= dwParam || m_pUser->m_oProfession.getProfessionMax() % 10 >= dwParam;
      }
      break;
    default:
      return false;
  }
  return true;
}

bool Servant::checkGrowthFinishCond(DWORD dwid, GrowthItemInfo* pItem)
{
  if(m_dwServantID == 0 || pItem == nullptr)
    return false;

  const SGrowthItemCFG* pItemCFG = ServantConfig::getMe().getGrowthCFG(dwid);
  if(pItemCFG == nullptr)
    return false;

  DWORD dwParam = 0;
  if(pItemCFG->stGrowthFinishCond.vecParams.empty() == false)
    dwParam = pItemCFG->stGrowthFinishCond.vecParams[0];

  switch (pItemCFG->stGrowthFinishCond.eTrigger)
  {
    case ETRIGGER_MIN:
      {
        return true;
      }
      break;
    case ETRIGGER_TOWER_PASS:
      {
        m_pUser->getTower().resetData();
        if(m_pUser->getTower().getCurMaxLayer() >= dwParam)
        {
          pItem->set_finishtimes(dwParam);
          return true;
        }
        return false;
      }
      break;
    case ETRIGGER_QUEST_SUBMIT:
      {
        return checkGrowthQuest(dwid, false, pItem, ETRIGGER_QUEST_SUBMIT);
      }
      break;
    case ETRIGGER_ACCEPT_QUEST:
      {
        return checkGrowthQuest(dwid, false, pItem, ETRIGGER_ACCEPT_QUEST);
      }
      break;
    case ETRIGGER_ITEM_GET:
      {
        DWORD dwCount = 1;
        if(pItemCFG->stGrowthFinishCond.vecParams.size() > 1)
          dwCount = pItemCFG->stGrowthFinishCond.vecParams[1];

        DWORD num = getItemCount(dwParam);
        if(num < dwCount)
          pItem->set_finishtimes(num);
        else
          pItem->set_finishtimes(dwCount);

        return num >= dwCount;
      }
      break;
    case ETRIGGER_OWN_TUTOR:
      {
        bool bAchieve = m_pUser->getAchieve().isFinishAchieve(1206009);
        return m_pUser->getSocial().getRelationCount(ESOCIALRELATION_TUTOR) >= dwParam || bAchieve;
      }
      break;
    case ETRIGGER_TITLE:
      {
        return m_pUser->getTitle().hasTitle(dwParam);
      }
      break;
    case ETRIGGER_MENU:
      {
        return checkGrowthMenu(dwid, false);
      }
      break;
    case ETRIGGER_JOIN_GUILD:
      {
        return m_pUser->hasGuild();
      }
      break;
    case ETRIGGER_CHANGE_PROFESSION:
      {
        return static_cast<DWORD>(m_pUser->getProfession() % 10) >= dwParam;
      }
      break;
    case ETRIGGER_MERCENARY_CAT:
      {
        return m_pUser->getWeaponPet().getList().size() >= dwParam;
      }
      break;
    case ETRIGGER_PHOTO_SCENERY:
      {
        if(m_pUser->getManual().getNumByStatus(EMANUALTYPE_SCENERY, EMANUALSTATUS_UNLOCK) >= dwParam)
        {
          pItem->set_finishtimes(dwParam);
          return true;
        }
        pItem->set_finishtimes(m_pUser->getManual().getNumByStatus(EMANUALTYPE_SCENERY, EMANUALSTATUS_UNLOCK));
      }
      break;
    case ETRIGGER_BATTLE_TIME:
      {
        m_pUser->antiAddictRefresh();
        DWORD dwRealTime = m_pUser->getBattleTime() + m_pUser->getUserSceneData().getUsedBattleTime();
        if(dwRealTime >= dwParam)
        {
          pItem->set_finishtimes(dwParam/60);
          return true;
        }
        pItem->set_finishtimes(dwRealTime/60);
      }
      break;
    case ETRIGGER_PRODUCE_HEAD:
      {
        if(m_pUser->getManual().getNumByStatus(EMANUALTYPE_FASHION, EMANUALSTATUS_UNLOCK) >= dwParam)
        {
          pItem->set_finishtimes(dwParam);
          return true;
        }
        pItem->set_finishtimes(m_pUser->getManual().getNumByStatus(EMANUALTYPE_FASHION, EMANUALSTATUS_UNLOCK));
      }
      break;
    case ETRIGGER_GUILD_PRAY:
      {
        return m_pUser->getGuildPrayTimes() >= dwParam;
      }
      break;
    case ETRIGGER_MAPID:
      {
        return m_pUser->getUserSceneData().isNewMap(dwParam) == false;
      }
      break;
    case ETRIGGER_OWN_STUDENT:
      {
        return m_pUser->getSocial().getRelationCount(ESOCIALRELATION_STUDENT) >= dwParam;
      }
      break;
    case ETRIGGER_OWN_PET:
      {
        return m_pUser->getManual().getNumByStatus(EMANUALTYPE_PET, EMANUALSTATUS_UNLOCK_CLIENT) >= dwParam;
      }
      break;
    case ETRIGGER_PET_FRIEND_FULL:
      {
        return m_pUser->getUserPet().getFirendFullNum() >= dwParam;
      }
      break;
    case ETRIGGER_PETWORK_SPACE:
      {
        return m_pUser->getPetWork().getMaxWorkCount() >= dwParam;
      }
      break;
    case ETRIGGER_GUILD_ARTIFACT:
      {
        return m_pUser->getGuild().getArtifactList().size() >= dwParam;
      }
      break;
    case ETRIGGER_GUILD_LEVEL:
      {
        return m_pUser->getGuild().lv() >= dwParam;
      }
      break;
    case ETRIGGER_SPACE_BREAK_SKILL:
      {
        return m_pUser->getCurFighter() && m_pUser->getCurFighter()->getSkill().isSkillEnable(dwParam);
      }
      break;
    case ETRIGGER_GUILD_BUILDING:
      {
        GuildScene* pScene = dynamic_cast<GuildScene*>(m_pUser->getScene());
        if(pScene == nullptr)
          return false;
        DWORD dwLv = 1;
        if(pItemCFG->stGrowthFinishCond.vecParams.size() > 1)
          dwLv = pItemCFG->stGrowthFinishCond.vecParams[1];
        return pScene->getGuild().getBuildingNum(dwLv) >= dwParam;
      }
      break;
    case ETRIGGER_BUY_PROFESSION:
      {
        return m_pUser->m_oProfession.getBuyBaseProfessionCount() >= dwParam;
      }
      break;
    case ETRIGGER_MANUAL_UNLOCK:
      {
        DWORD dwNum = 1;
        if(pItemCFG->stGrowthFinishCond.vecParams.size() > 1)
          dwNum = pItemCFG->stGrowthFinishCond.vecParams[1];
        return m_pUser->getManual().getNumByStatus(static_cast<EManualType>(dwParam), EMANUALSTATUS_UNLOCK) >= dwNum;
      }
      break;
    case ETRIGGER_GRADUATE:
      {
        return m_pUser->getAchieve().isFinishAchieve(1206009);
      }
      break;
    case ETRIGGER_POLLY_FIGHT:
    case ETRIGGER_CAT_INVASION:
    case ETRIGGER_GVG:
    case ETRIGGER_GUILD_FUBEN:
    case ETRIGGER_PET_ADVENTURE:
    case ETRIGGER_PETWORK:
    case ETRIGGER_TUTOR_SKILL:
    case ETRIGGER_AUGURY:
    case ETRIGGER_WANTED_QUEST_DAY:
    case ETRIGGER_DAILYMONSTER:
    case ETRIGGER_REPAIR_SEAL:
    case ETRIGGER_LABORATORY:
    case ETRIGGER_GUILD_DONATE:
    case ETRIGGER_LISTEN_MUSIC:
    case ETRIGGER_PLAY_MUSIC:
    case ETRIGGER_PVP_KILL:
    case ETRIGGER_EAT_FOOD:
    case ETRIGGER_DOJO:
    case ETRIGGER_ENCHANT_PRIMARY:
    case ETRIGGER_ENCHANT_MIDDLE:
    case ETRIGGER_ENCHANT_ADVANCED:
    case ETRIGGER_COOKFOOD:
    case ETRIGGER_HAIR:
    case ETRIGGER_EYE:
    case ETRIGGER_LOTTERY:
    case ETRIGGER_GUILD_BUILD_DONATE:
    case ETRIGGER_WORLD_FREYJA:
    case ETRIGGER_CARD_RESET:
    case ETRIGGER_CARD_CUSTOMIZE:
    case ETRIGGER_ENCHANT_HEAD:
    case ETRIGGER_KILL_MINI:
    case ETRIGGER_KILL_MVP:
    case ETRIGGER_PVE_CARD:
    case ETRIGGER_KILL_STAR_NPC:
    case ETRIGGER_MVP_BATTLE:
    case ETRIGGER_EXCHANGE_HEAD_DRAWING:
    case ETRIGGER_HEAD_COMPOSE:
    case ETRIGGER_EQUIP_COMPOSE:
    case ETRIGGER_EQUIP_COMPOSE_WEAPON:
    case ETRIGGER_EQUIP_COMPOSE_ARMOUR:
    case ETRIGGER_EQUIP_REFINE:
    case ETRIGGER_SAFE_REFINE:
    case ETRIGGER_ENTER_PVP:
    case ETRIGGER_CARD_MOSAIC:
    case ETRIGGER_USE_CATAPULT:
    case ETRIGGER_EXTRACT_CAT_LITTER:
    case ETRIGGER_VENDING_MACHINE:
    case ETRIGGER_EQUIP_UPGRADE:
    case ETRIGGER_GUILD_SPECIAL_PRAY:
    case ETRIGGER_GUILD_HEAD_REFINE:
    case ETRIGGER_GUILD_EQUIP_FIRM:
    case ETRIGGER_CASTLE_RAID:
    case ETRIGGER_EQUIP_DECOMPOSE:
    case ETRIGGER_EQUIP_STRENGTH:
    case ETRIGGER_MVP_BATTLE_KILL:
    case ETRIGGER_MVP_RECORD_SAVE:
    case ETRIGGER_MVP_RECORD_LOAD:
    case ETRIGGER_PROFESSION_EXCHANGE:
    case ETRIGGER_PVE_CARD_SIMPLE:
    case ETRIGGER_PVE_CARD_MIDDLE:
    case ETRIGGER_FLAME_RAID:
    case ETRIGGER_EQUIP_RESTORE:
    case ETRIGGER_EQUIP_EXCHANGE:
    case ETRIGGER_PET_ADVENTURE_REWARD:
    case ETRIGGER_FRIENDSHIP_MORACOIN:
    case ETRIGGER_START_PETWORK:
    case ETRIGGER_GOLD_MEDAL:
    case ETRIGGER_NIGHTMARE_RAID:
    case ETRIGGER_TERRORIST_RAID:
    case ETRIGGER_LUN_SHARD:
    case ETRIGGER_LUN_SHARD_SPECIAL:
    case ETRIGGER_GVG_DEFENSE:
    case ETRIGGER_GUILD_QUEST_ACCEPT:
    case ETRIGGER_GUILD_QUEST_SUBMIT:
    case ETRIGGER_TRADE_MATERIAL:
    case ETRIGGER_PET_CAPTURE:
    case ETRIGGER_QUICK_WANTED_QUEST:
    case ETRIGGER_CAPRA:
    case ETRIGGER_BIG_LOTTERY:
      {
        if(pItem->finishtimes() >= dwParam)
        {
          pItem->set_finishtimes(dwParam);
          return true;
        }
      }
      break;
    default:
      return false;
  }

  return false;
}

bool Servant::onGrowthAppearEvent(ETriggerType eType)
{
  if(m_dwServantID == 0)
    return false;

  for(auto &it : m_mapGrowthItems)
  {
    if(checkExistCurGroup(it.first / SERVANT_GROWTH_ID_PARAM) == false)
      continue;
    if(ServantConfig::getMe().getGrowthAppearTrigger(it.first) != eType)
      continue;

    if(it.second.status() == EGROWTH_STATUS_MIN && checkGrowthAppearCond(it.first) == true)
    {
      setGrowthStatus(&(it.second), EGROWTH_STATUS_GO);
      if(checkGrowthFinishCond(it.first, &(it.second)) == true)
        setGrowthStatus(&(it.second), EGROWTH_STATUS_RECEIVE);
      refreshGrowthInfo(it.first, false);
    }
  }

  return true;
}

bool Servant::onGrowthFinishEvent(ETriggerType eType, DWORD dwParam)
{
  if(m_dwServantID == 0)
    return false;

  for(auto &it : m_mapGrowthItems)
  {
    if(checkExistCurGroup(it.first / SERVANT_GROWTH_ID_PARAM) == false)
      continue;
    if(it.second.status() != EGROWTH_STATUS_GO)
      continue;
    const SGrowthItemCFG* pItemCFG = ServantConfig::getMe().getGrowthCFG(it.first);
    if(pItemCFG == nullptr || pItemCFG->stGrowthFinishCond.eTrigger != eType)
      continue;

    DWORD dwTimes = it.second.finishtimes();
    if(isCountTriggerType(pItemCFG->stGrowthFinishCond.eTrigger) == true)
      it.second.set_finishtimes(dwTimes + dwParam);
    if(checkGrowthFinishCond(it.first, &(it.second)) == true)
      setGrowthStatus(&(it.second), EGROWTH_STATUS_RECEIVE);
    if(dwTimes != it.second.finishtimes() || it.second.status() == EGROWTH_STATUS_RECEIVE)
      refreshGrowthInfo(it.first, false);
  }

  return true;
}

void Servant::getGrowthReward(DWORD dwid, DWORD dwvalue)
{
  if(dwid == 0)
  {
    const SGrowthItemCFG* pItemCFG = ServantConfig::getMe().getGrowthCFG(dwvalue);
    if(pItemCFG == nullptr)
      return;

    GrowthItemInfo* pItem = getGrowthItem(dwvalue);
    if(pItem == nullptr || pItem->status() != EGROWTH_STATUS_RECEIVE)
      return;

    setGrowthStatus(pItem, EGROWTH_STATUS_FINISH);
    refreshGrowthInfo(dwvalue, true);

    if(pItemCFG->dwGrowth != 0)
      addGrowthValue(pItemCFG->dwGroupID, pItemCFG->dwGrowth);
    if(pItemCFG->dwReward != 0)
      m_pUser->getPackage().rollReward(pItemCFG->dwReward, EPACKMETHOD_AVAILABLE, false, true);

    onGrowthAppearEvent(ETRIGGER_LAST_GROWTH);
    XLOG <<  "[仆人-提升], 领奖" << m_pUser->name << m_pUser->id << "提升ID:" << dwvalue << "Growth: " << pItemCFG->dwGrowth
      << "rewardID: " << pItemCFG->dwReward << XEND;
  }
  else        //成长值奖励
  {
    auto it = m_mapGrowthValues.find(dwid);
    if(it == m_mapGrowthValues.end() || it->second.growth() < dwvalue)
      return;

    DWORD dwRewardID = MiscConfig::getMe().getServantCFG().getGrowthRewardID(dwid, dwvalue);
    if(dwRewardID == 0)
      return;

    for(int i = 0; i < it->second.everreward_size(); ++i)
    {
      if(dwvalue == it->second.everreward(i))
        return;
    }
    it->second.add_everreward(dwvalue);
    m_pUser->getPackage().rollReward(dwRewardID, EPACKMETHOD_AVAILABLE, false, true);
    sendGrowthStatus(it->first);
    XLOG <<  "[仆人-成长值奖励], 玩家" << m_pUser->name << m_pUser->id << "成长计划:" << dwid << "成长值:" << dwvalue << dwRewardID << XEND;
  }

  if(canAddGrowthRedTip() == true)
    m_pUser->getTip().addRedTip(EREDSYS_SERVANT_GROWTH);
  else
    m_pUser->getTip().removeRedTip(EREDSYS_SERVANT_GROWTH);
}

bool Servant::addGrowthValue(DWORD dwGroup, DWORD dwValue)
{
  auto it = m_mapGrowthValues.find(dwGroup);
  if(it != m_mapGrowthValues.end())
  {
    it->second.set_growth(it->second.growth() + dwValue);
  }
  else
  {
    Cmd::GrowthValueInfo info;
    info.set_groupid(dwGroup);
    info.set_growth(dwValue);
    m_mapGrowthValues.emplace(dwGroup, info);
  }

  sendGrowthStatus(dwGroup);
  return true;
}

bool Servant::setGrowthStatus(GrowthItemInfo* pItem, EGrowthStatus eStatus, bool bCheck)
{
  if(pItem == nullptr)
    return false;

  const SGrowthItemCFG* pItemCFG = ServantConfig::getMe().getGrowthCFG(pItem->dwid());
  if(pItemCFG == nullptr)
    return false;

  if(pItem->status() + 1 == eStatus || bCheck == false)
  {
    pItem->set_status(eStatus);
    if(eStatus == EGROWTH_STATUS_RECEIVE)
      m_pUser->getTip().addRedTip(EREDSYS_SERVANT_GROWTH);
    else if(eStatus == EGROWTH_STATUS_FINISH)
      m_setUnlockedGrowth.emplace(pItem->dwid());
    XDBG << "[仆人-提升状态]" << m_pUser->id << m_pUser->name << "提升ID: " << pItem->dwid() << "状态: " << eStatus  << bCheck << XEND;
    return true;
  }

  return false;
}

void Servant::refreshGrowthInfo(DWORD dwid, bool addUnlock)
{
  GrowthServantUserCmd cmd;
  GrowthGroupInfo *pData = cmd.add_datas();
  if(pData == nullptr)
    return;

  GrowthItemInfo *pItem = pData->add_items();
  GrowthItemInfo *pGrowthItem = getGrowthItem(dwid);
  if(pItem == nullptr || pGrowthItem == nullptr)
    return;

  pItem->set_dwid(dwid);
  pItem->set_finishtimes(pGrowthItem->finishtimes());
  pItem->set_status(pGrowthItem->status());

  if(addUnlock == true)
    cmd.add_unlockitems(dwid);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

GrowthItemInfo* Servant::getGrowthItem(DWORD dwid)
{
  auto it = m_mapGrowthItems.find(dwid);
  if(it != m_mapGrowthItems.end())
    return &it->second;

  return nullptr;
}

void Servant::sendGrowthStatus(DWORD dwGroupID)
{
  Cmd::GrowthServantUserCmd cmd;
  GrowthGroupInfo *pData = cmd.add_datas();
  if(pData == nullptr)
    return;

  auto it = m_mapGrowthValues.find(dwGroupID);
  if(it != m_mapGrowthValues.end())
  {
    Cmd::GrowthValueInfo *pItem = pData->mutable_valueitems();
    if(pItem == nullptr)
      return;

    pItem->set_groupid(dwGroupID);
    pItem->set_growth(it->second.growth());
    for(int i = 0; i < it->second.everreward_size(); ++i)
      pItem->add_everreward(it->second.everreward(i));
  }

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void Servant::initDefaultGrowthData()
{
  if(m_mapGrowthItems.empty() == true)
  {
    const TMapGrowthCFG mapGrowthCFG = ServantConfig::getMe().getAllGrowthCFG();
    for(auto m = mapGrowthCFG.begin(); m != mapGrowthCFG.end(); ++m)
    {
      auto it = m_mapGrowthItems.find(m->first);
      if(it != m_mapGrowthItems.end())
        continue;
      GrowthItemInfo sItem;
      sItem.set_dwid(m->first);

      if(checkGrowthGroupAppearCond(m->first) == true)
      {
        m_mapCurGrowthGroup.emplace(static_cast<EGrowthType>(m->second.dwType), m->second.dwGroupID);
        resetGrowthData(&sItem);
      }

      m_mapGrowthItems.emplace(m->first, sItem);
    }
  }
  else
  {
    for(auto m = m_mapGrowthItems.begin(); m != m_mapGrowthItems.end(); ++m)
    {
      if(checkExistCurGroup(m->first / SERVANT_GROWTH_ID_PARAM) == true)
        resetGrowthData(&m->second);
    }
  }

  sendAllGrowthInfo();
}

void Servant::resetGrowthData(GrowthItemInfo* pItem)
{
  if(pItem == nullptr)
    return;

  const SGrowthItemCFG* pItemCFG = ServantConfig::getMe().getGrowthCFG(pItem->dwid());
  if(pItemCFG == nullptr)
    return;

  if(pItem->status() == EGROWTH_STATUS_MIN && checkGrowthAppearCond(pItem->dwid()) == true)
    setGrowthStatus(pItem, EGROWTH_STATUS_GO);
  if(pItem->status() == EGROWTH_STATUS_GO && checkGrowthFinishCond(pItem->dwid(), pItem) == true)
    setGrowthStatus(pItem, EGROWTH_STATUS_RECEIVE);

  if(pItem->status() == EGROWTH_STATUS_FINISH)
    m_setUnlockedGrowth.emplace(pItem->dwid());
}

bool Servant::openNewGrowth(DWORD groupid)
{
  bool bOpen = false;
  auto it = m_mapCurGrowthGroup.begin();
  for(; it != m_mapCurGrowthGroup.end(); ++it)
  {
    if(groupid == it->second)
    {
      bOpen = true;
      break;
    }
  }
  if(bOpen == false)
    return bOpen;

  if(checkGrowthGroupFinishCond(groupid) == false)
    return false;

  if(checkExistGroup(groupid + 1))
  {
    it->second = groupid + 1;
    for(auto m = m_mapGrowthItems.begin(); m != m_mapGrowthItems.end(); ++m)
    {
      if(m->first / SERVANT_GROWTH_ID_PARAM == groupid + 1)
        resetGrowthData(&m->second);
    }

    sendGrowthInfo(groupid + 1);
    if(canAddGrowthRedTip() == false)
      m_pUser->getTip().removeRedTip(EREDSYS_SERVANT_GROWTH);
  }

  XLOG << "[仆人-成长计划开启]" << m_pUser->id << m_pUser->name << "GroupID: " << groupid + 1 << XEND;
  return true;
}

bool Servant::canAddGrowthRedTip()
{
  for(auto &it : m_mapCurGrowthGroup)
  {
    DWORD groupid = it.second;
    if(checkGrowthGroupFinishCond(groupid) == true && checkExistGroup(groupid + 1))
      return true;

    DWORD dwCanGetRewardNum = MiscConfig::getMe().getServantCFG().getCanGrowthRewardNum(groupid, m_mapGrowthValues[groupid].growth());
    DWORD dwGetRewardNum = m_mapGrowthValues[groupid].everreward_size();
    if(dwCanGetRewardNum > dwGetRewardNum)
      return true;

    for(auto &s : m_mapGrowthItems)
    {
      DWORD group = s.first / SERVANT_GROWTH_ID_PARAM;
      if(group != groupid)
        continue;

      if(s.second.status() == EGROWTH_STATUS_RECEIVE)
        return true;
    }
  }

  return false;
}

bool Servant::checkExistGroup(DWORD groupid)
{
  auto it = find_if(m_mapGrowthItems.begin(), m_mapGrowthItems.end(), [&](const pair<DWORD, GrowthItemInfo>& v) -> bool
      { return v.first / SERVANT_GROWTH_ID_PARAM == groupid; });
  if(it != m_mapGrowthItems.end())
    return true;

  return false;
}

bool Servant::checkExistCurGroup(DWORD groupid)
{
  auto it = find_if(m_mapCurGrowthGroup.begin(), m_mapCurGrowthGroup.end(), [&](const pair<EGrowthType, DWORD>& v) -> bool
      { return v.second == groupid; });
  if(it != m_mapCurGrowthGroup.end())
    return true;

  return false;
}

EGrowthStatus Servant::getGrowthStatus(DWORD dwid)
{
  EGrowthStatus eRet = EGROWTH_STATUS_MIN;

  auto it = m_mapGrowthItems.find(dwid);
  if(it != m_mapGrowthItems.end())
    eRet = it->second.status();

  return eRet;
}

bool Servant::checkGrowthQuest(DWORD dwid, bool isAppearEvent, GrowthItemInfo* pItem, ETriggerType type)
{
  const SGrowthItemCFG* pItemCFG = ServantConfig::getMe().getGrowthCFG(dwid);
  if(pItemCFG == nullptr)
    return false;
  if(isAppearEvent == true)
  {
    if(pItemCFG->stGrowthAppearCond.vecParams.empty() == true || pItemCFG->stGrowthAppearCond.eTrigger != type)
      return false;
    if(pItemCFG->stGrowthAppearCond.vecParams.size() == 1)
    {
      if(type == ETRIGGER_QUEST_SUBMIT)
        return m_pUser->getQuest().isSubmit(pItemCFG->stGrowthAppearCond.vecParams[0]);
      else if(type == ETRIGGER_ACCEPT_QUEST)
        return m_pUser->getQuest().isAccept(pItemCFG->stGrowthAppearCond.vecParams[0]) || m_pUser->getQuest().isSubmit(pItemCFG->stGrowthAppearCond.vecParams[0]);
    }
    else if(pItemCFG->stGrowthAppearCond.vecParams.size() > 1)
    {
      if(pItemCFG->stGrowthAppearCond.vecParams[0] == 0)    //或
      {
        for(auto it = pItemCFG->stGrowthAppearCond.vecParams.begin(); it != pItemCFG->stGrowthAppearCond.vecParams.end(); ++it)
        {
          if(it == pItemCFG->stGrowthAppearCond.vecParams.begin())
            continue;
          if(type == ETRIGGER_QUEST_SUBMIT && m_pUser->getQuest().isSubmit(*it) == true)
            return true;
          else if(type == ETRIGGER_ACCEPT_QUEST && (m_pUser->getQuest().isAccept(*it) || m_pUser->getQuest().isSubmit(*it)))
            return true;
        }
        return false;
      }
      else    //与
      {
        for(auto it = pItemCFG->stGrowthAppearCond.vecParams.begin(); it != pItemCFG->stGrowthAppearCond.vecParams.end(); ++it)
        {
          if(it == pItemCFG->stGrowthAppearCond.vecParams.begin())
            continue;
          if(type == ETRIGGER_QUEST_SUBMIT && m_pUser->getQuest().isSubmit(*it) == false)
            return false;
          else if(type == ETRIGGER_ACCEPT_QUEST && (m_pUser->getQuest().isAccept(*it) == false && m_pUser->getQuest().isSubmit(*it) == false))
            return false;
        }
        return true;
      }
    }
  }
  else
  {
    if(pItemCFG->stGrowthFinishCond.vecParams.empty() == true || pItemCFG->stGrowthFinishCond.eTrigger != type)
      return false;
    if(pItemCFG->stGrowthFinishCond.vecParams.size() == 1)
    {
      if(type == ETRIGGER_QUEST_SUBMIT)
        return m_pUser->getQuest().isSubmit(pItemCFG->stGrowthFinishCond.vecParams[0]);
      else if(type == ETRIGGER_ACCEPT_QUEST)
        return m_pUser->getQuest().isAccept(pItemCFG->stGrowthFinishCond.vecParams[0]) || m_pUser->getQuest().isSubmit(pItemCFG->stGrowthFinishCond.vecParams[0]);
    }
    else if(pItemCFG->stGrowthFinishCond.vecParams.size() > 1)
    {
      if(pItemCFG->stGrowthFinishCond.vecParams[0] == 0)    //或
      {
        for(auto it = pItemCFG->stGrowthFinishCond.vecParams.begin(); it != pItemCFG->stGrowthFinishCond.vecParams.end(); ++it)
        {
          if(it == pItemCFG->stGrowthFinishCond.vecParams.begin())
            continue;
          if(type == ETRIGGER_QUEST_SUBMIT && m_pUser->getQuest().isSubmit(*it) == true)
            return true;
          else if(type == ETRIGGER_ACCEPT_QUEST && (m_pUser->getQuest().isAccept(*it) || m_pUser->getQuest().isSubmit(*it)))
            return true;
        }
        return false;
      }
      else    //与
      {
        DWORD dwNum = 0;
        for(DWORD i = 1; i < pItemCFG->stGrowthFinishCond.vecParams.size(); ++i)
        {
          if(type == ETRIGGER_QUEST_SUBMIT && m_pUser->getQuest().isSubmit(pItemCFG->stGrowthFinishCond.vecParams[i]) == true)
            dwNum++;
          else if(type == ETRIGGER_ACCEPT_QUEST && (m_pUser->getQuest().isAccept(pItemCFG->stGrowthFinishCond.vecParams[i]) || m_pUser->getQuest().isSubmit(pItemCFG->stGrowthFinishCond.vecParams[i])))
            dwNum++;
        }
        if(pItem != nullptr && dwNum > pItem->finishtimes())
        {
          pItem->set_finishtimes(dwNum);
          refreshGrowthInfo(dwid, false);
        }
        if(dwNum >= pItemCFG->stGrowthFinishCond.vecParams.size() - 1)
          return true;
        return false;
      }
    }
  }

  return false;
}

bool Servant::checkGrowthMenu(DWORD dwid, bool isAppearEvent)
{
  const SGrowthItemCFG* pItemCFG = ServantConfig::getMe().getGrowthCFG(dwid);
  if(pItemCFG == nullptr)
    return false;
  if(isAppearEvent == true)
  {
    if(pItemCFG->stGrowthAppearCond.eTrigger != ETRIGGER_MENU || pItemCFG->stGrowthAppearCond.vecParams.empty() == true)
      return false;

    if(pItemCFG->stGrowthAppearCond.vecParams.size() == 1)
      return m_pUser->getMenu().isOpen(pItemCFG->stGrowthAppearCond.vecParams[0]);
    else if(pItemCFG->stGrowthAppearCond.vecParams.size() > 1)
    {
      if(pItemCFG->stGrowthAppearCond.vecParams[0] == 0)    //或
      {
        for(auto it = pItemCFG->stGrowthAppearCond.vecParams.begin(); it != pItemCFG->stGrowthAppearCond.vecParams.end(); ++it)
        {
          if(it == pItemCFG->stGrowthAppearCond.vecParams.begin())
            continue;
          if(m_pUser->getMenu().isOpen(*it) == true)
            return true;
        }
        return false;
      }
      else    //与
      {
        for(auto it = pItemCFG->stGrowthAppearCond.vecParams.begin(); it != pItemCFG->stGrowthAppearCond.vecParams.end(); ++it)
        {
          if(it == pItemCFG->stGrowthAppearCond.vecParams.begin())
            continue;
          if(m_pUser->getMenu().isOpen(*it) == false)
            return false;
        }
        return true;
      }
    }
  }
  else
  {
    if(pItemCFG->stGrowthFinishCond.eTrigger != ETRIGGER_MENU || pItemCFG->stGrowthFinishCond.vecParams.empty() == true)
      return false;

    if(pItemCFG->stGrowthFinishCond.vecParams.size() == 1)
      return m_pUser->getMenu().isOpen(pItemCFG->stGrowthFinishCond.vecParams[0]);
    else if(pItemCFG->stGrowthFinishCond.vecParams.size() > 1)
    {
      if(pItemCFG->stGrowthFinishCond.vecParams[0] == 0)    //或
      {
        for(auto it = pItemCFG->stGrowthFinishCond.vecParams.begin(); it != pItemCFG->stGrowthFinishCond.vecParams.end(); ++it)
        {
          if(it == pItemCFG->stGrowthFinishCond.vecParams.begin())
            continue;
          if(m_pUser->getMenu().isOpen(*it) == true)
            return true;
        }
        return false;
      }
      else    //与
      {
        for(auto it = pItemCFG->stGrowthFinishCond.vecParams.begin(); it != pItemCFG->stGrowthFinishCond.vecParams.end(); ++it)
        {
          if(it == pItemCFG->stGrowthFinishCond.vecParams.begin())
            continue;
          if(m_pUser->getMenu().isOpen(*it) == false)
            return false;
        }
        return true;
      }
    }
  }

  return false;
}

void Servant::checkNewGrowthGroup()
{
  if(m_dwServantID == 0)
    return;

  DWORD dwGroup = 0;
  for (auto m = m_mapGrowthItems.begin(); m != m_mapGrowthItems.end(); ++m)
  {
    const SGrowthItemCFG* pItemCFG = ServantConfig::getMe().getGrowthCFG(m->first);
    if(pItemCFG == nullptr)
      continue;

    if(dwGroup == 0)
    {
      auto it = m_mapCurGrowthGroup.find(static_cast<EGrowthType>(pItemCFG->dwType));
      if(it != m_mapCurGrowthGroup.end())
        continue;

      if(checkGrowthGroupAppearCond(m->first) == true)
      {
        dwGroup = pItemCFG->dwGroupID;
        resetGrowthData(&m->second);
        m_mapCurGrowthGroup.emplace(static_cast<EGrowthType>(pItemCFG->dwType), pItemCFG->dwGroupID);
      }
    }
    else if(dwGroup == pItemCFG->dwGroupID)
    {
      resetGrowthData(&m->second);
    }
  }

  if(dwGroup != 0)
    sendAllGrowthInfo();
}

bool Servant::isCountTriggerType(ETriggerType type)
{
  switch (type)
  {
    case ETRIGGER_QUEST_SUBMIT:
    case ETRIGGER_MENU:
    case ETRIGGER_ITEM_GET:
    case ETRIGGER_LEVELUP:
      return false;
    default:
      return true;
  }

  return true;
}

void Servant::finishGrowthGroup(DWORD dwGroupID)
{
  auto it = find_if(m_mapCurGrowthGroup.begin(), m_mapCurGrowthGroup.end(), [&](const pair<EGrowthType, DWORD>& v) -> bool
      { return v.second == dwGroupID; });
  if(it == m_mapCurGrowthGroup.end())
    return;

  for(auto &s : m_mapGrowthItems)
  {
    if(s.first / SERVANT_GROWTH_ID_PARAM != dwGroupID)
      continue;

    setGrowthStatus(&s.second, EGROWTH_STATUS_RECEIVE, false);
    refreshGrowthInfo(s.first, false);
  }
}

DWORD Servant::getItemCount(DWORD dwItemid)
{
  DWORD num = 0;
  MainPackage* pPackage = dynamic_cast<MainPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_MAIN));
  if (pPackage)
    num += pPackage->getItemCount(dwItemid);
  BarrowPackage* pBarrowPack = dynamic_cast<BarrowPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_BARROW));
  if (pBarrowPack)
    num += pBarrowPack->getItemCount(dwItemid);
  PersonalStorePackage* pPersonalStorePack = dynamic_cast<PersonalStorePackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_PERSONAL_STORE));
  if (pPersonalStorePack)
    num += pPersonalStorePack->getItemCount(dwItemid);
  BasePackage* pFoodPack = m_pUser->getPackage().getPackage(EPACKTYPE_FOOD);
  if (pFoodPack)
    num += pFoodPack->getItemCount(dwItemid);

  return num;
}
