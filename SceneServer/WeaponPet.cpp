#include "WeaponPet.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "SceneNpc.h"
#include "SceneNpcManager.h"
#include "Scene.h"
#include "WeaponPetConfig.h"

void SWeaponPetData::toData(WeaponPetData* pData)
{
  if (pData == nullptr)
    return;
  pData->set_id(dwID);
  pData->set_hp(dwHp);
  pData->set_relivetime(dwReliveTime);
  pData->set_expiretime(dwExpireTime);
  pData->set_blive(bLive);
  pData->set_bactive(bActive);
  pData->set_handstatus(bInHandStatus);
}

void SWeaponPetData::fromData(const WeaponPetData& rData)
{
  dwID = rData.id();
  dwHp = rData.hp();
  dwReliveTime = rData.relivetime();
  dwExpireTime = rData.expiretime();
  bLive = rData.blive();
  bActive = rData.bactive();
  bInvalid = dwExpireTime <= now();
  bInHandStatus = rData.handstatus();

  const SWeaponPetCFG* pCFG = WeaponPetConfig::getMe().getCFG(dwID);
  if (pCFG)
    strName = pCFG->name;
}

void SWeaponPetData::toData(MemberCat* pCat, SceneUser* pUser) const
{
  if (pCat == nullptr || pUser == nullptr)
    return;

  pCat->set_ownerid(pUser->id);
  pCat->set_id(getTempID(pUser->id));
  pCat->set_catid(dwID);
  pCat->set_relivetime(dwReliveTime);
  pCat->set_lv(pUser->getLevel());
  pCat->set_expiretime(dwExpireTime);
  pCat->set_name(strName);
}

void SWeaponPetData::toData(TeamMember* pMember, SceneUser* pUser) const
{
  if (pMember == nullptr || pUser == nullptr)
    return;

  QWORD qwTempID = getTempID(pUser->id);
  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(qwTempID);
  if (pNpc != nullptr)
  {
    GTeam::add_mdata(pMember->add_datas(), EMEMBERDATA_HP, pNpc->getAttr(EATTRTYPE_HP));
    GTeam::add_mdata(pMember->add_datas(), EMEMBERDATA_MAXHP, pNpc->getAttr(EATTRTYPE_MAXHP));
    pMember->set_guid(pNpc->id);
  }
  else
  {
    GTeam::add_mdata(pMember->add_datas(), EMEMBERDATA_HP, dwHp);
    GTeam::add_mdata(pMember->add_datas(), EMEMBERDATA_MAXHP, 100);
    pMember->set_guid(getTempID(pUser->id));
  }

  GTeam::add_mdata(pMember->add_datas(), EMEMBERDATA_BASELEVEL, pUser->getLevel());
  GTeam::add_mdata(pMember->add_datas(), EMEMBERDATA_CAT, dwID);
  GTeam::add_mdata(pMember->add_datas(), EMEMBERDATA_CAT_OWNER, pUser->id);
  GTeam::add_mdata(pMember->add_datas(), EMEMBERDATA_RELIVETIME, dwReliveTime);
  GTeam::add_mdata(pMember->add_datas(), EMEMBERDATA_EXPIRETIME, dwExpireTime);
  GTeam::add_mdata(pMember->add_datas(), EMEMBERDATA_NAME, 0, strName);
}

WeaponPet::WeaponPet(SceneUser* pUser) : m_pUser(pUser)
{

}

WeaponPet::~WeaponPet()
{

}

void WeaponPet::load(const BlobWeaponPet& data)
{
  m_oListPetData.clear();
  for (int i = 0; i < data.datas_size(); ++i)
  {
    SWeaponPetData stData;
    stData.fromData(data.datas(i));
    m_oListPetData.push_back(stData);
  }
  m_dwMaxPetSize = data.maxpetsize();
  m_setUnlockIDs.clear();
  for (int i = 0; i < data.unlockids_size(); ++i)
  {
    m_setUnlockIDs.insert(data.unlockids(i));
  }
}

void WeaponPet::save(BlobWeaponPet* pPet)
{
  pPet->clear_datas();
  for (auto &s : m_oListPetData)
  {
    WeaponPetData* pData = pPet->add_datas();
    if (pData == nullptr)
      continue;
    s.toData(pData);
  }
  pPet->set_maxpetsize(m_dwMaxPetSize);
  pPet->clear_unlockids();
  for (auto &s : m_setUnlockIDs)
  {
    pPet->add_unlockids(s);
  }
}

void WeaponPet::toData(MemberCatUpdateTeam& cmd)
{
  for (auto &s : m_oListPetData)
    s.toData(cmd.add_updates(), m_pUser);
}

void WeaponPet::setMaxSize(DWORD size)
{
  if (size == m_dwMaxPetSize)
    return;
  const SSystemCFG& syscfg = MiscConfig::getMe().getSystemCFG();
  size  = size > syscfg.dwMaxWeaponPetNum ? syscfg.dwMaxWeaponPetNum : size;
  m_dwMaxPetSize = size;
  m_pUser->getServant().onFinishEvent(ETRIGGER_UNLOCK_CATNUM);
  XLOG << "[战斗猫-解锁], 玩家:" << m_pUser->name << m_pUser->id << "当前数量" << m_dwMaxPetSize << XEND;
}

void WeaponPet::unlock(DWORD id)
{
  if (m_setUnlockIDs.find(id) != m_setUnlockIDs.end())
    return;
  m_setUnlockIDs.insert(id);
  XLOG << "[战斗猫-解锁], 玩家:" << m_pUser->name << m_pUser->id << "解锁战斗猫ID:" << id << XEND;
}

bool WeaponPet::checkUnConflict(DWORD id) const
{
  const SWeaponPetCFG* pCFG = WeaponPetConfig::getMe().getCFG(id);
  if (pCFG == nullptr)
    return false;
  for (auto &s : m_oListPetData)
  {
    if (s.bInvalid)
      continue;
    //if (s.dwID == id)   //可以续约正在战斗的佣兵猫
    //  return false;
    if (pCFG->setConflictIDs.find(s.dwID) != pCFG->setConflictIDs.end())
      return false;
    const SWeaponPetCFG* pAddCFG = WeaponPetConfig::getMe().getCFG(s.dwID);
    if (pAddCFG && pAddCFG->setConflictIDs.find(id) != pAddCFG->setConflictIDs.end())
      return false;
  }
  return true;
}

bool WeaponPet::checkCanAdd(DWORD id) const
{
  if (checkUnlock(id) == false)
    return false;
  if (checkUnConflict(id) == false)
    return false;
  if (checkSize(id) == false)
    return false;
  return true;
}

bool WeaponPet::checkAddMoney(DWORD id, EEmployType eType, bool& bNeedZeny, DWORD&dwZenyCount, DWORD& dwCostItemId, DWORD& dwCostItemNum)
{
  const SWeaponPetCFG* pPetCFG = WeaponPetConfig::getMe().getCFG(id);
  if (pPetCFG == nullptr)
  {
    XERR << "[战斗猫-添加], 找不到配置" << id << "玩家:" << m_pUser->name << m_pUser->id << XEND;
    return false;
  }
  if (eType <= EEMPLOYTYPE_MIN || eType >= EEMPLOYTYPE_MAX)
    return false;
  auto m = pPetCFG->mapType2Money.find(eType);
  if (m == pPetCFG->mapType2Money.end())
  {
    XERR << "[战斗猫-添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "雇佣" << id << eType << "猫失败,不正确的type类型" << XEND;
    return false;
  }

  bNeedZeny = true;
  DWORD costItem = MiscConfig::getMe().getItemCFG().dwWeponcatItem;
    BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if(pMainPack == nullptr)
    return false;
  DWORD itemNum = pMainPack->getItemCount(costItem);

  if (eType == EEMPLOYTYPE_DAY)
    {
    if(itemNum >= 1)
    {
      bNeedZeny = false;
      dwZenyCount = 0;
      dwCostItemId = costItem;
      dwCostItemNum = 1;
    }
    else
    {
      bNeedZeny = true;
      dwZenyCount = m->second;
      dwCostItemId = 0;
      dwCostItemNum = 0;
    }
  }
  else if (eType == EEMPLOYTYPE_WEEK)
  {
    if(itemNum >= 7)
    {
      bNeedZeny = false;
      dwZenyCount = 0;
      dwCostItemId = costItem;
      dwCostItemNum = 7;
    }
    else if(itemNum > 0)
    {
      auto it = pPetCFG->mapType2Money.find(EEMPLOYTYPE_DAY);
      if (it == pPetCFG->mapType2Money.end())
      {
        XERR << "[战斗猫-添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "雇佣" << id << "猫失败,未找到每日消耗" << XEND;
        return false;
  }

      bNeedZeny = true;
      dwZenyCount = it->second * pPetCFG->dwDiscount * (7 - itemNum) / 100;
      dwCostItemId = costItem;
      dwCostItemNum = itemNum;
    }
    else
  {
      bNeedZeny = true;
      dwZenyCount = m->second;
      dwCostItemId = 0;
      dwCostItemNum = 0;
    }
  }
  else
    return false;

  if (bNeedZeny && m_pUser->checkMoney(EMONEYTYPE_SILVER, dwZenyCount) == false)
  {
    XERR << "[战斗猫-添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "雇佣" << id << eType << "猫失败,zeny不足,需要" << dwZenyCount << "zeny" << XEND;
    return false;
  }

  return true;
}

bool WeaponPet::add(DWORD id, EEmployType eType)
{
  if (!m_pUser)
    return false;
  const SWeaponPetCFG* pPetCFG = WeaponPetConfig::getMe().getCFG(id);
  if (pPetCFG == nullptr)
  {
    XERR << "[战斗猫-添加], 找不到配置" << id << "玩家:" << m_pUser->name << m_pUser->id << XEND;
    return false;
  }
  if (eType <= EEMPLOYTYPE_MIN || eType >= EEMPLOYTYPE_MAX)
    return false;

  bool bNeedZeny = true;
  DWORD dwZenyCount = 0;
  DWORD dwCostItemId = 0;
  DWORD dwCostItemNum = 0;
  if (checkAddMoney(id, eType, bNeedZeny, dwZenyCount, dwCostItemId, dwCostItemNum) == false)
  {
    MsgManager::sendMsg(m_pUser->id, 4204);
    return false;
  }

  DWORD npcid = pPetCFG->dwMonsterID;
  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(npcid);
  if (pCFG == nullptr)
    return false;
  if (pCFG->eNpcType != ENPCTYPE_WEAPONPET)
    return false;

  if (checkCanAdd(id) == false)
  {
    XERR << "[战斗猫-添加], 当前战斗猫数不可添加" << id << "玩家:" << m_pUser->name << m_pUser->id << XEND;
    MsgManager::sendMsg(m_pUser->id, 4203);
    return false;
  }

  if (dwCostItemId)
  {
    BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
    if (pMainPack)
      pMainPack->reduceItem(dwCostItemId, ESOURCE_CAT, dwCostItemNum);
  }
  if (bNeedZeny)
    m_pUser->subMoney(EMONEYTYPE_SILVER, dwZenyCount, ESOURCE_CAT);

  if(dwCostItemId && bNeedZeny)
    MsgManager::sendMsg(m_pUser->id, 4202, MsgParams(dwCostItemNum, dwZenyCount));
  else if(dwCostItemId)
    MsgManager::sendMsg(m_pUser->id, 4200, MsgParams(dwCostItemNum));
  else if(bNeedZeny)
    MsgManager::sendMsg(m_pUser->id, 4201, MsgParams(dwZenyCount));

  bool isClientReHire = false;
  auto itold = find_if(m_oListPetData.begin(), m_oListPetData.end(), [&](SWeaponPetData& r) ->bool {
      return r.dwID == id; //&& r.bInvalid == true;
      });
  if (itold == m_oListPetData.end() || itold->bInvalid == true)
  {
    if(itold != m_oListPetData.end())
    {
    m_oListPetData.erase(itold);
    isClientReHire = true;
  }

  if (pPetCFG->dwReleatedNpcID)
    m_pUser->addHideNpc(pPetCFG->dwReleatedNpcID);

  SWeaponPetData stPet;
  stPet.dwID = id;
  stPet.bLive = true;
  stPet.bActive = false; // 等待进队成功消息
  stPet.setMark(EMEMBERDATA_EXPIRETIME);
  stPet.strName = pPetCFG->name;

  switch(eType)
  {
    case EEMPLOYTYPE_MIN:
    case EEMPLOYTYPE_MAX:
      break;
    case EEMPLOYTYPE_DAY:
      stPet.dwExpireTime = now() + DAY_T;
      break;
    case EEMPLOYTYPE_WEEK:
      stPet.dwExpireTime = now() + WEEK_T;
      break;
    default:
      stPet.dwExpireTime = now();
      break;
  }
  m_oListPetData.push_back(stPet);
  onAdd(stPet);
  m_pUser->getServant().onFinishEvent(ETRIGGER_MERCENARY_CAT);
  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_MERCENARY_CAT);

  // 立即同步猫状态, 灰色图标变亮
  if (isClientReHire)
    updateDataToTeam();
    XLOG << "[战斗猫], 玩家召唤战斗猫, 玩家:" << m_pUser->name << m_pUser->id << "战斗猫:" << npcid << "雇佣时间(类型):" << (DWORD)eType << isClientReHire << XEND;
  }
  else if (itold->bInvalid == false)
  {
    switch(eType)
    {
      case EEMPLOYTYPE_MIN:
      case EEMPLOYTYPE_MAX:
        break;
      case EEMPLOYTYPE_DAY:
        itold->dwExpireTime += DAY_T;
        break;
      case EEMPLOYTYPE_WEEK:
        itold->dwExpireTime += WEEK_T;
        break;
      default:
        itold->dwExpireTime = now();
        break;
    }
    itold->setMark(EMEMBERDATA_EXPIRETIME);
    updateDataToTeam();
    XLOG << "[战斗猫], 玩家续约战斗猫, 玩家:" << m_pUser->name << m_pUser->id << "战斗猫:" << npcid << "雇佣时间(类型):" << (DWORD)eType << XEND;
  }

  return true;
}

bool WeaponPet::del(DWORD id)
{
  for (auto &s : m_oListPetData)
  {
    if (s.dwID == id)
    {
      s.bClearStatus = true;
      //s.dwExpireTime = cur;
    }
  }
  return true;
}

bool WeaponPet::enable(DWORD id)
{
  if (checkCanActive(id) == false)
  {
    XLOG << "[战斗猫-出场失败], 已出场足够数量, 当前出场数量" << m_dwMaxPetSize << "玩家:" << m_pUser->name << m_pUser->id;
    return false;
  }
  for (auto &s : m_oListPetData)
  {
    if (s.dwID != id)
      continue;
    if (s.bActive)
      continue;
    s.bActive = true;
    enterScene(s);
  }
  return true;
}

bool WeaponPet::disable(DWORD id, bool bToRest /*=true*/)
{
  for (auto &s : m_oListPetData)
  {
    if (s.dwID != id)
      continue;
    if (s.bActive == false)
      continue;
    s.bActive = false;
    leaveScene(s);
    if (bToRest)
    {
      enterRest(s);
    }
  }
  return true;
}

bool WeaponPet::hasActive() const
{
  for (auto &l : m_oListPetData)
  {
    if (l.bActive)
      return true;
  }
  return false;
}

const SWeaponPetData* WeaponPet::getData(DWORD dwID) const
{
  for (auto &l : m_oListPetData)
  {
    if (l.dwID == dwID)
      return &l;
  }

  return nullptr;
}

void WeaponPet::onUserMove(const xPos& dest)
{
  if (!m_pUser->getScene() || m_oListPetData.empty())
    return;
  //if (getMoveList(dest, list) == false)
    //return;

  DWORD index = 0;
  for (auto &s : m_oListPetData)
  {
    if (s.notIn())
      continue;
    switch(s.eState)
    {
      case EWPPETSTATAE_NORAML:
      case EWPPETSTATAE_WAIT_BATTLE:
        break;
      case EWPPETSTATAE_TEAMER:
        {
          if (m_pUser->isMyTeamMember(s.qwFollowTeamerID) == false)
          {
            changeState(s, EWPPETSTATAE_WAIT_BATTLE);
            break;
          }
          SceneUser* pTeamer = SceneUserManager::getMe().getUserByID(s.qwFollowTeamerID);
          if (!pTeamer || pTeamer->getScene() != m_pUser->getScene() || getDistance(pTeamer->getPos(), m_pUser->getPos()) > MiscConfig::getMe().getWeaponPetCFG().dwHelpTeamerDis)
            changeState(s, EWPPETSTATAE_WAIT_BATTLE);
          break;
        }
      case EWPPETSTATAE_HAND:
        continue;
        break;
      default:
        break;
    }

    index ++;
    onMove(dest, s, index);
  }
}

void WeaponPet::onUserMoveTo(const xPos& dest)
{
  for (auto &s : m_oListPetData)
  {
    if (s.eState != EWPPETSTATAE_HAND || s.notIn())
      continue;
    SceneNpc* hander = SceneNpcManager::getMe().getNpcByTempID(s.getTempID(m_pUser->id));
    if (hander)
      hander->setScenePos(dest);
  }
}

void WeaponPet::onUserAttack(xSceneEntryDynamic* enemy)
{
  if (!enemy)
    return;
  // 玩家切线后 猫不会攻击boss or mini
  if (m_pUser->isJustInViceZone())
  {
    SceneNpc* pNpc = dynamic_cast<SceneNpc*> (enemy);
    if (pNpc && pNpc->getNpcZoneType() == ENPCZONE_FIELD)
    {
      if (pNpc->getNpcType() == ENPCTYPE_MVP || pNpc->getNpcType() == ENPCTYPE_MINIBOSS)
        return;
    }
  }
  DWORD cur = now();
  for (auto &s : m_oListPetData)
  {
    if (s.notIn())
      continue;
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(s.getTempID(m_pUser->id));
    if (!npc)
      continue;

    switch(s.eState)
    {
      case EWPPETSTATAE_NORAML:
        break;
      case EWPPETSTATAE_WAIT_BATTLE:
      case EWPPETSTATAE_HAND:
        changeState(s, EWPPETSTATAE_NORAML);
        break;
      case EWPPETSTATAE_TEAMER:
        if (npc->m_ai.getCurLockID() != enemy->id)
          changeState(s, EWPPETSTATAE_NORAML);
        break;
      default:
        break;
    }

    npc->m_ai.setCurLockID(enemy->id);
    s.dwTempEndBattleTime = cur + 3;
    s.dwLastBattleTime = cur;
  }
}

void WeaponPet::onTeamerAttack(SceneUser* pTeamer, xSceneEntryDynamic* enemy)
{
  if (pTeamer == nullptr || enemy == nullptr)
    return;
  //if (m_pUser->getUserSceneData().getFollowerID() == 0)
    //return;

  if (getDistance(m_pUser->getPos(), pTeamer->getPos()) > MiscConfig::getMe().getWeaponPetCFG().dwHelpTeamerDis)
    return;

  if (pTeamer->isJustInViceZone())
  {
    SceneNpc* pNpc = dynamic_cast<SceneNpc*> (enemy);
    if (pNpc)
    {
      if (pNpc->getNpcType() == ENPCTYPE_MVP || pNpc->getNpcType() == ENPCTYPE_MINIBOSS)
        return;
    }
  }

  DWORD cur = now();
  for (auto &s : m_oListPetData)
  {
    if (s.notIn())
      continue;
    switch(s.eState)
    {
      case EWPPETSTATAE_NORAML:
        break;
      case EWPPETSTATAE_WAIT_BATTLE:
      case EWPPETSTATAE_HAND:
        changeState(s, EWPPETSTATAE_TEAMER);
        s.qwFollowTeamerID = pTeamer->id;
        break;
      case EWPPETSTATAE_TEAMER:
        {
          if (s.qwFollowTeamerID != 0 && s.qwFollowTeamerID != pTeamer->id) // 已经跟随其他队友
            break;
          SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(s.getTempID(m_pUser->id));
          if (!npc)
            continue;
          npc->m_ai.setCurLockID(enemy->id);
          s.dwTempEndBattleTime = cur + 3;
          s.dwLastBattleTime = cur;
        }
        break;
      default:
        break;
    }
  }
}

void WeaponPet::onUserEnterScene()
{
  for (auto &s : m_oListPetData)
  {
    enterScene(s);
  }
  /*
  if (m_pUser->getTeamID() == 0)
    onEnterOwnTeam();
  else
    checkTeam();
    */
  if (m_pUser->getTeamID() != 0)
    checkTeam();

  m_pUser->getAchieve().onCat(true);
}

void WeaponPet::onUserLeaveScene()
{
  for (auto &s : m_oListPetData)
  {
    leaveScene(s);
  }
}

void WeaponPet::enterScene(SWeaponPetData& stData)
{
  Scene* pScene = m_pUser->getScene();
  if (pScene == nullptr)
    return;
  if (stData.notIn())
    return;
  // 部分地图 不可携带
  if (pScene->getBaseCFG() && pScene->getBaseCFG()->noCat())
    return;

  const SWeaponPetCFG* pPetCFG = WeaponPetConfig::getMe().getCFG(stData.dwID);
  if (pPetCFG == nullptr)
  {
    XERR << "[战斗猫-进入场景], 找不到配置" << stData.dwID << "玩家:" << m_pUser->name << m_pUser->id << XEND;
    return;
  }
  DWORD npcid = pPetCFG->dwMonsterID;
  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(npcid);
  if (pCFG == nullptr || pCFG->eNpcType != ENPCTYPE_WEAPONPET)
    return;

  xPos pos = m_pUser->getPos();
  m_pUser->getScene()->getRandPos(m_pUser->getPos(), 5, pos);

  NpcDefine define;
  define.m_oVar.m_qwOwnerID = m_pUser->id;
  define.setPos(pos);
  define.setID(pCFG->dwID);
  define.setLife(1);
  define.setBehaviours(define.getBehaviours() & ~BEHAVIOUR_OUT_RANGE_BACK);
  define.setTerritory(0);
  define.setWeapnPetID(stData.dwID);
  if (stData.dwHp != 0)
    define.m_oVar.m_dwDefaultHp = stData.dwHp;

  SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(define, m_pUser->getScene());
  if (pNpc == nullptr)
    return;

  stData.dwHp = pNpc->getAttr(EATTRTYPE_HP);
  stData.eState = EWPPETSTATAE_WAIT_BATTLE;
  if (stData.bInHandStatus)
    sendHandMark(pNpc->id);

  XDBG << "[战斗猫-进入场景], 玩家:" << m_pUser->name << m_pUser->id << "地图:" << m_pUser->getScene()->id << "战斗猫:" << pNpc->name << pNpc->id << stData.dwID << XEND;
}

void WeaponPet::leaveScene(SWeaponPetData& stData)
{
  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(stData.getTempID(m_pUser->id));
  if (pNpc == nullptr)
    return;
  if (pNpc->define.m_oVar.m_qwOwnerID != m_pUser->id)
    return;
  stData.dwHp = pNpc->getAttr(EATTRTYPE_HP);
  pNpc->removeAtonce();

  if (m_pUser->getScene())
    XDBG << "[战斗猫-离开场景], 玩家:" << m_pUser->name << m_pUser->id << "地图:" << m_pUser->getScene()->id << "战斗猫:" << pNpc->name << pNpc->id << stData.dwID << XEND;
}


void WeaponPet::onNpcDie(SceneNpc* npc)
{
  if (npc == nullptr)
    return;
  auto it = find_if(m_oListPetData.begin(), m_oListPetData.end(), [&](const SWeaponPetData& r) ->bool {
      return r.getTempID(m_pUser->id) == npc->id && r.bLive;
      });
  if (it == m_oListPetData.end())
    return;
  enterRest(*it);

  XLOG << "[战斗猫-死亡], 玩家:" << m_pUser->name << m_pUser->id << "战斗猫:" << it->dwID << XEND;
}

void WeaponPet::enterRest(SWeaponPetData& stData)
{
  const SWeaponPetCFG* pCFG = WeaponPetConfig::getMe().getCFG(stData.dwID);
  if (pCFG == nullptr)
    return;

  stData.bLive = false;
  stData.dwReliveTime = now() + pCFG->dwReliveTime;
  stData.dwHp = 0;
  changeState(stData, EWPPETSTATAE_WAIT_BATTLE);

  stData.setMark(EMEMBERDATA_RELIVETIME);
  stData.setMark(EMEMBERDATA_HP);
  stData.setMark(EMEMBERDATA_MAXHP);
  updateDataToTeam();

  XLOG << "[战斗猫-进入休整期], 玩家:" << m_pUser->name << m_pUser->id << "战斗猫:" << stData.dwID << XEND;
}

void WeaponPet::checkTeam()
{
  if (m_pUser == nullptr)
    return;
  if (m_pUser->getTeamID() == 0)
    return;

  GTeam& rTeam = m_pUser->getTeam();
  for (auto &m : rTeam.getTeamMemberList())
  {
    if (m.second.catid() == 0)
      continue;
    if (getData(m.second.catid()) == nullptr)
    {
      CatFireTeamCmd cmd;
      cmd.set_charid(m_pUser->id);
      cmd.set_npcid(m.second.charid());
      cmd.set_catid(m.second.catid());
      PROTOBUF(cmd, send, len);
      thisServer->sendCmdToSession(send, len);
      XLOG << "[战斗猫-成员检查]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "战斗猫 :" << m.second.catid() << "不在列表中,主动离队" << XEND;
    }
  }
}

void WeaponPet::onDel(SWeaponPetData& stData, bool bRealDel /*=false*/)
{
  leaveScene(stData);

  const SWeaponPetCFG* pCFG = WeaponPetConfig::getMe().getCFG(stData.dwID);
  if (pCFG && pCFG->dwReleatedNpcID)
    m_pUser->delHideNpc(pCFG->dwReleatedNpcID);

  if (bRealDel)
  {
    // inform team
    MemberCatUpdateTeam cmd;
    MemberCat* pCat = cmd.add_dels();
    stData.toData(pCat, m_pUser);
    PROTOBUF(cmd, send, len);
    broadcastCmdToTeam(send, len);

    if (m_pUser->getTeamID() != 0)
    {
      CatFireTeamCmd fcmd;
      fcmd.set_charid(m_pUser->id);
      fcmd.set_npcid(stData.getTempID(m_pUser->id));
      fcmd.set_catid(stData.dwID);
      PROTOBUF(fcmd, fsend, flen);
      thisServer->sendCmdToSession(fsend, flen);
    }
    else
    {
      TeamMemberUpdate cmd;
      cmd.add_deletes(stData.getTempID(m_pUser->id));
      PROTOBUF(cmd, send, len);
      m_pUser->sendCmdToMe(send, len);
    }
  }
}

void WeaponPet::onRlive(SWeaponPetData& stData)
{
  if (m_pUser == nullptr)
    return;

  stData.setMark(EMEMBERDATA_RELIVETIME);
  stData.setMark(EMEMBERDATA_HP);
  stData.setMark(EMEMBERDATA_MAXHP);
  updateDataToTeam();
}

void WeaponPet::timer(DWORD curSec)
{
  for (auto s = m_oListPetData.begin(); s != m_oListPetData.end(); )
  {
    if (s->bClearStatus)
    {
      XLOG << "[战斗猫-删除], 玩家:" << m_pUser->name << m_pUser->id << "战斗猫:" << s->dwID << XEND;
      if (s->bInHandStatus)
      {
        s->bInHandStatus = false;
        sendHandBreak(s->getTempID(m_pUser->id));
      }
      onDel(*s, true);
      s = m_oListPetData.erase(s);
      continue;
    }
    if (s->bInvalid)
    {
      ++s;
      continue;
    }
    if (s->dwExpireTime != 0 && curSec >= s->dwExpireTime)
    {
      XLOG << "[战斗猫-删除], 玩家:" << m_pUser->name << m_pUser->id << "战斗猫:" << s->dwID << XEND;
      onDel(*s);
      //s = m_oListPetData.erase(s);
      s->bInvalid = true;
      s->setMark(EMEMBERDATA_EXPIRETIME);
      updateDataToTeam();
      if (s->bInHandStatus)
      {
        s->bInHandStatus = false;
        sendHandBreak(s->getTempID(m_pUser->id));
      }
      ++s;
      continue;
    }

    if (s->bActive)
    {
      // 复活
      if (s->bLive == false && curSec >= s->dwReliveTime)
      {
        s->bLive = true;
        enterScene(*s);
        onRlive(*s);
        XLOG << "[战斗猫-复活], 玩家:" << m_pUser->name << m_pUser->id << "战斗猫:" << s->dwID << XEND;
      }

      // 停止战斗状态
      if (s->bLive && s->dwTempEndBattleTime && curSec >= s->dwTempEndBattleTime)
      {
        s->dwTempEndBattleTime = 0;
        SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(s->getTempID(m_pUser->id));
        if (npc && npc->define.m_oVar.m_qwOwnerID == m_pUser->id)
        {
          if (npc->m_ai.getCurLockID() != 0)
            npc->m_ai.setCurLockID(0);
        }
      }

      // user die, check something to do
      if (s->bLive && m_pUser->isAlive() == false)
      {
        SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(s->getTempID(m_pUser->id));
        if (npc && npc->define.m_oVar.m_qwOwnerID == m_pUser->id)
        {
          npc->m_sai.checkSig("employer_die_status");
        }
      }
    }
    else
    {
      if (s->bLive == false && curSec >= s->dwReliveTime)
      {
        s->bLive = true;
        XLOG << "[战斗猫-复活], 玩家:" << m_pUser->name << m_pUser->id << "战斗猫:" << s->dwID << XEND;
      }
    }

    // 切换状态, 跟随自己or队友战斗
    if (s->bActive && s->bLive)
    {
      switch(s->eState)
      {
        case EWPPETSTATAE_NORAML:
          if (curSec > s->dwLastBattleTime + MiscConfig::getMe().getWeaponPetCFG().dwOffOwnerToBattleTime)
          {
            changeState(*s, EWPPETSTATAE_WAIT_BATTLE);
          }
          break;
        case EWPPETSTATAE_WAIT_BATTLE:
          {
            // 建立牵手
            if (s->bInHandStatus)
            {
              SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(s->getTempID(m_pUser->id));
              if (npc)
              {
                if (npc->m_oMove.empty())
                  npc->m_ai.moveTo(m_pUser->getPos());
                if (getXZDistance(npc->getPos(), m_pUser->getPos()) < 2 && npc->getAction() == 0)
                  changeState(*s, EWPPETSTATAE_HAND);
              }
            }
            break;
          }
        case EWPPETSTATAE_TEAMER:
          if (curSec > s->dwLastBattleTime + MiscConfig::getMe().getWeaponPetCFG().dwOffTeamerToBattleTime)
          {
            changeState(*s, EWPPETSTATAE_WAIT_BATTLE);
          }
          break;
        case EWPPETSTATAE_HAND:
          break;
        default:
          break;
      }
    }

    ++s;
  }
}

/*
bool WeaponPet::getMoveList(const xPos& dest, std::list<xPos>& poslist)
{
  if (!m_pUser->getScene())
    return false;
  DWORD activesize = 0;
  for (auto &s : m_oListPetData)
  {
    if (s.bLive == false || s.bActive == false)
      continue;
    ++activesize;
  }
  if (activesize == 0)
    return false;
  poslist.clear();

  std::list<xPos> list;
  if (m_pUser->getScene()->findingPath(dest, m_pUser->getPos(), list, TOOLMODE_PATHFIND_FOLLOW) == false)
    return false;
  if (list.size() <= 2)
    return false;

  list.pop_front();
  list.pop_front();
  xPos dest1 = list.front();

  auto getPosOnCircle = [&](const xPos& pos0, const xPos& pos1, float angle, xPos& out)
  {
    angle = angle * 3.14 / 180.0f;
    out = pos0;
    out.z = pos0.z + (pos1.z - pos0.z) * cos(angle) - (pos1.x - pos0.x) * sin(angle);
    out.x = pos0.x + (pos1.z - pos0.z) * sin(angle) + (pos1.x - pos0.x) * cos(angle);
  };
  switch(activesize)
  {
    case 1 :
      poslist.push_back(dest1);
      break;
    case 2 :
      {
        xPos tmppos;
        getPosOnCircle(dest, dest1, 30, tmppos);
        poslist.push_back(tmppos);
        getPosOnCircle(dest, dest1, -30, tmppos);
        poslist.push_back(tmppos);
      }
      break;
    case 3 :
      {
        xPos tmppos = dest1;
        poslist.push_back(tmppos);
        getPosOnCircle(dest, dest1, 45, tmppos);
        poslist.push_back(tmppos);
        getPosOnCircle(dest, dest1, -45, tmppos);
        poslist.push_back(tmppos);
      }
      break;
    default:
      return false;
      break;
  }
  return true;
}
*/

bool WeaponPet::onMove(const xPos& dest, const SWeaponPetData& stData, DWORD index)
{
  if (!m_pUser->getScene())
    return false;
  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(stData.getTempID(m_pUser->id));
  if (!npc || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
    return false;
  npc->m_ai.setCurLockID(0);

  // 已经标记需要牵手, 并且处于未战斗状态, 移动处理:紧追玩家
  if (stData.bInHandStatus && stData.eState == EWPPETSTATAE_WAIT_BATTLE)
  {
    npc->m_ai.moveTo(dest);
    return true;
  }

  const SWeaponPetCFG* pCFG = WeaponPetConfig::getMe().getCFG(stData.dwID);
  if (pCFG == nullptr)
    return false;
  if (getDistance(dest, npc->getPos()) < (float)(pCFG->dwFollowDis))
    return false;

  DWORD allsize = 0;
  for (auto &s : m_oListPetData)
  {
    if (s.notIn() == false)
      allsize ++;
  }
  if (index > allsize)
    return false;

  std::list<xPos> list;
  if (m_pUser->getScene()->findingPath(dest, m_pUser->getPos(), list, TOOLMODE_PATHFIND_FOLLOW) == false)
    return false;
  if (list.size() < 2)
    return false;
  list.pop_front();
  xPos dest1 = list.front();

  xPos mydest = dest1;
  float foldis = pCFG->dwFollowDis;
  float dis = getDistance(dest, dest1);
  if (foldis > dis && dis > 0)
  {
    mydest.x = foldis / dis * (dest1.x - dest.x) + dest.x;
    mydest.z = foldis / dis * (dest1.z - dest.z) + dest.z;
  }
  /*
  DWORD dis = pCFG->dwFollowDis;
  for (DWORD i = 0; i < dis; ++i)
  {
    if (list.empty())
      return false;
    list.pop_front();
  }
  if (list.empty())
    return false;
  xPos mydest = list.front();
    */

  auto getPosOnCircle = [&](const xPos& pos0, const xPos& pos1, float angle, xPos& out)
  {
    angle = angle * 3.14 / 180.0f;
    out = pos0;
    out.z = pos0.z + (pos1.z - pos0.z) * cos(angle) - (pos1.x - pos0.x) * sin(angle);
    out.x = pos0.x + (pos1.z - pos0.z) * sin(angle) + (pos1.x - pos0.x) * cos(angle);
  };
  xPos outpos;
  switch(allsize)
  {
    case 1 :
      outpos = mydest;
      break;
    case 2 :
      outpos = mydest;
      if (index == 1)
        getPosOnCircle(dest, mydest, 30, outpos);
      else if (index == 2)
        getPosOnCircle(dest, mydest, -30, outpos);
      break;
    case 3 :
      outpos = mydest;
      if (index == 2)
        getPosOnCircle(dest, mydest, 45, outpos);
      if (index == 3)
        getPosOnCircle(dest, mydest, -45, outpos);
      break;
    case 4:
      outpos = mydest;
      if (index == 1)
        getPosOnCircle(dest, mydest, 18, outpos);
      if (index == 2)
        getPosOnCircle(dest, mydest, -18, outpos);
      if (index == 3)
        getPosOnCircle(dest, mydest, 54, outpos);
      if (index == 4)
        getPosOnCircle(dest, mydest, -54, outpos);
      break;
    default:
      return false;
      break;
  }
  if (!outpos.empty())
    npc->m_ai.moveTo(outpos);
  return true;
}

void WeaponPet::onUserDie()
{
  for (auto &s : m_oListPetData)
  {
    if (s.notIn())
      continue;
    changeState(s, EWPPETSTATAE_WAIT_BATTLE);

    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(s.getTempID(m_pUser->id));
    if (!npc)
      continue;
    if (npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
      continue;
    npc->m_sai.checkSig("employer_die");
  }
}

void WeaponPet::onUserRelive()
{
  for (auto &s : m_oListPetData)
  {
    if (s.notIn())
      continue;
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(s.getTempID(m_pUser->id));
    if (!npc)
      continue;
    if (npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
      continue;
    npc->m_sai.checkSig("employer_relive");
  }
}

void WeaponPet::onAdd(const SWeaponPetData& rData)
{
  if (checkCanActive(rData.dwID) == false)
    return;

  MemberCatUpdateTeam cmd;
  MemberCat* pCat = cmd.add_updates();
  rData.toData(pCat, m_pUser);
  PROTOBUF(cmd, send, len);

  if (m_pUser->getTeamID() != 0)
  {
    thisServer->broadcastOneLevelIndexCmd(ONE_LEVEL_INDEX_TYPE_TEAM, m_pUser->getTeamID(), send, len);

    bool alreadyInTeam = m_pUser->getTeamMember(rData.getTempID(m_pUser->id)) != nullptr;
    if (!alreadyInTeam)
    {
      CatEnterTeamCmd cmd;
      cmd.set_charid(m_pUser->id);
      rData.toData(cmd.add_cats(), m_pUser);
      if (cmd.cats_size() > 0)
      {
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToSession(send, len);
      }
    }
    else
    {
      enable(rData.dwID);
    }
  }
  else
  {
    enable(rData.dwID);
    onEnterOwnTeam();
  }
}

void WeaponPet::onEnterOwnTeam(bool isLeaveTeam/*=false*/)
{
  if (m_pUser == nullptr || has() == false)
    return;

  if (m_pUser->getTeam().getTeamID() != 0)
  {
    XERR << "[佣兵猫-自身队伍]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "进入自身队伍失败,已有真的队伍" << XEND;
    return;
  }

  CatEnterOwnTeamCmd cmd;
  TeamMember* pData = cmd.mutable_data()->add_members();
  if (pData == nullptr)
  {
    XERR << "[佣兵猫-自身队伍]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "进入自身队伍失败,创建自身成员失败" << XEND;
    return;
  }
  pData->set_guid(m_pUser->id);
  pData->set_name(m_pUser->name);
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_BASELEVEL, m_pUser->getLevel());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_PORTRAIT, m_pUser->getPortrait().getCurPortrait());

  const TListPetData& listData = getList();
  for (auto &l : listData)
  {
    if (!l.bActive)
      continue;
    // 原先在队的猫进队, 保持在队
    l.toData(cmd.mutable_data()->add_members(), m_pUser);
  }

  // 队伍变化, 把可进队的猫自动进队
  if (isLeaveTeam)
  {
    for (auto &l : listData)
    {
      if (l.bActive)
        continue;

      // 优先未到期猫进队
      if (l.bInvalid)
        continue;

      if (enable(l.dwID) == false)
        continue;
      l.toData(cmd.mutable_data()->add_members(), m_pUser);
    }
    for (auto &l : listData)
    {
      if (l.bActive)
        continue;

      // 到期猫后进队
      if (l.bInvalid == false)
        continue;

      if (enable(l.dwID) == false)
        continue;
      l.toData(cmd.mutable_data()->add_members(), m_pUser);
    }
  }

  if (cmd.data().members_size() > 1)
  {
    cmd.set_charid(m_pUser->id);
    PROTOBUF(cmd, send, len);;
    thisServer->sendCmdToSession(send, len);
  }
}

void WeaponPet::onEnterTeam(bool isEnterTeam, bool isOnline)
{
  if (m_pUser == nullptr)
    return;

  CatEnterTeamCmd cmd;
  cmd.set_charid(m_pUser->id);

  // 1. 优先之前在队的猫 2. 优先未到期的猫
  std::list<DWORD> disIDs;
  if (isEnterTeam)
  {
    // 记录顺序, 优先之前在队的猫
    for (auto &l : m_oListPetData)
    {
      if (l.bActive)
      {
        // 优先未到期的猫
        if (l.bInvalid == false)
          disIDs.push_front(l.dwID);
        else
          disIDs.push_back(l.dwID);
      }
    }

    std::list<DWORD> disIDs2;
    for (auto &l : m_oListPetData)
    {
      if (!l.bActive)
      {
        // 优先未到期的猫
        if (l.bInvalid)
          disIDs2.push_back(l.dwID);
        else
          disIDs2.push_front(l.dwID);

        continue;
      }
      disable(l.dwID, false);
    }

    for (auto &s : disIDs2)
      disIDs.push_back(s);
  }
  else if (isOnline)
  {
    for (auto &l : m_oListPetData)
    {
      if (l.bActive)
      {
        disable(l.dwID, false);
        disIDs.push_back(l.dwID);
      }
    }
  }

  for (auto &l : m_oListPetData)
  {
    if (m_pUser->getTeamMember(l.getTempID(m_pUser->id)) != nullptr)
    {
      l.setMark(EMEMBERDATA_BASELEVEL);
      l.setMark(EMEMBERDATA_RELIVETIME);
      l.setMark(EMEMBERDATA_EXPIRETIME);
      l.setMark(EMEMBERDATA_HP);
      l.setMark(EMEMBERDATA_MAXHP);

      if (isOnline && l.bActive == false && enable(l.dwID) == false) // 切线后可能不一致
      {
        KickMember cmd;
        cmd.set_userid(l.getTempID(m_pUser->id));
        cmd.set_catid(l.dwID);
        PROTOBUF(cmd, send, len);
        thisServer->forwardCmdToSessionUser(m_pUser->id, send, len);
      }

      updateDataToTeam();
    }
  }

  DWORD index = 0;
  for (auto &s : disIDs)
  {
    if (index >= m_dwMaxPetSize)
      break;
    auto l = find_if(m_oListPetData.begin(), m_oListPetData.end(), [&](const SWeaponPetData& r) -> bool{
        return s == r.dwID;
        });
    if (l == m_oListPetData.end())
      continue;
    l->toData(cmd.add_cats(), m_pUser);
    ++index;
  }

  if (cmd.cats_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
  }
}

void WeaponPet::onEnterTeamFail(DWORD id)
{
  for (auto &l : m_oListPetData)
  {
    if (l.dwID != id)
      continue;
    if (l.bInHandStatus)
      breakHand(l.getTempID(m_pUser->id));
  }
}

void WeaponPet::onUserGoTo(const xPos& dest)
{
  for (auto &s : m_oListPetData)
  {
    if (s.notIn())
      continue;
    switch(s.eState)
    {
      case EWPPETSTATAE_NORAML:
        break;
      case EWPPETSTATAE_WAIT_BATTLE:
      case EWPPETSTATAE_TEAMER:
        changeState(s, EWPPETSTATAE_NORAML);
        break;
      case EWPPETSTATAE_HAND:
        break;
      default:
        break;
    }

    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(s.getTempID(m_pUser->id));
    if (npc == nullptr || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
      continue;
    xPos pos = dest;
    if (m_pUser->getScene())
      m_pUser->getScene()->getRandPos(m_pUser->getPos(), 5, pos);
    npc->m_oMove.clear();
    npc->goTo(pos);
  }
}

void WeaponPet::getPetNpcs(std::list<SceneNpc*>& list) const
{
  for (auto &s : m_oListPetData)
  {
    if (s.notIn())
      continue;
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(s.getTempID(m_pUser->id));
    if (!npc || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
      continue;
    list.push_back(npc);
  }
}

void WeaponPet::broadcastCmdToTeam(void* buf, WORD len)
{
  if (buf == nullptr || len == 0)
    return;

  if (m_pUser->getTeam().getTeamID() == 0)
    m_pUser->sendCmdToMe(buf, len);
  else
    thisServer->broadcastOneLevelIndexCmd(ONE_LEVEL_INDEX_TYPE_TEAM, m_pUser->getTeam().getTeamID(), buf, len);
}

void WeaponPet::updateDataToTeam()
{
  if (m_pUser == nullptr)
    return;

  for (auto &l : m_oListPetData)
  {
    if (l.obitset.any() == false)
      continue;

    if (m_pUser->getTeamID() != 0 && m_pUser->getTeamMember(l.getTempID(m_pUser->id)) == nullptr)
    {
      l.obitset.reset();
      continue;
    }

    MemberDataUpdate cmd;
    fetchTeamData(l, cmd);
    if (cmd.members_size() > 0)
    {
      PROTOBUF(cmd, send, len);
      broadcastCmdToTeam(send, len);
    }
    l.obitset.reset();

#ifdef _DEBUG
    for (int i = 0; i < cmd.members_size(); ++i)
    {
      const MemberData& rData = cmd.members(i);
      XDBG << "[战斗猫-数据更新-组队]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "队伍 :" << m_pUser->getTeamID() << "猫 :" << l.dwID << "更新 type :" << rData.type() << "value :" << rData.value() << XEND;
    }
#endif
  }
}

void WeaponPet::updateDataToUser(QWORD qwTargetID)
{
  for (auto &l : m_oListPetData)
  {
    if (l.obitset.any() == false)
      continue;

    if (m_pUser->getTeamID() != 0 && m_pUser->getTeamMember(l.getTempID(m_pUser->id)) == nullptr)
    {
      l.obitset.reset();
      continue;
    }

    MemberDataUpdate cmd;
    fetchTeamData(l, cmd);
    if (cmd.members_size() > 0)
    {
      PROTOBUF(cmd, send, len);
      thisServer->sendCmdToMe(qwTargetID, send, len);
    }
    l.obitset.reset();

#ifdef _DEBUG
    for (int i = 0; i < cmd.members_size(); ++i)
    {
      const MemberData& rData = cmd.members(i);
      XDBG << "[战斗猫-数据更新-个人]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "队伍 :" << m_pUser->getTeamID() << "猫 :" << l.dwID << "更新" << rData.ShortDebugString() << XEND;
    }
#endif
  }
}

void WeaponPet::setMark(DWORD id, EMemberData eData)
{
  for (auto &l : m_oListPetData)
  {
    if (l.dwID == id)
    {
      l.setMark(eData);
      break;
    }
  }
}

void WeaponPet::queryCatPrice(QueryOtherData& cmd)
{
  if (m_pUser == nullptr)
    return;

  const SWeaponPetCFG* pPetCFG = WeaponPetConfig::getMe().getCFG(cmd.data().param1());
  if (pPetCFG == nullptr)
  {
    XERR << "[战斗猫-价格查询]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "查询" << cmd.ShortDebugString() << "失败,未在Table_MercenaryCat.txt表中找到" << XEND;
    return;
  }
  DWORD dwType = cmd.data().param2();
  if (dwType <= EEMPLOYTYPE_MIN || dwType >= EEMPLOYTYPE_MAX)
  {
    XERR << "[战斗猫-价格查询]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "查询" << cmd.ShortDebugString() << "失败,type类型不合法" << XEND;
    return;
  }
  auto m = pPetCFG->mapType2Money.find(static_cast<EEmployType>(dwType));
  if (m == pPetCFG->mapType2Money.end())
  {
    XERR << "[战斗猫-价格查询]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "查询" << cmd.ShortDebugString() << "失败,type未在配置中找到" << XEND;
    return;
  }

  cmd.mutable_data()->set_param3(m->second);
  cmd.mutable_data()->set_param4(pPetCFG->dwDiscount);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XDBG << "[战斗猫-价格查询]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "查询成功,结果为" << cmd.ShortDebugString() << XEND;
}

DWORD WeaponPet::getMaxExpireTime() const
{
  DWORD dwTime = 0;
  for (auto &l : m_oListPetData)
  {
    if (l.dwExpireTime > dwTime)
      dwTime = l.dwExpireTime;
  }
  return dwTime;
}

void WeaponPet::fetchTeamData(SWeaponPetData& rData, MemberDataUpdate& cmd)
{
  cmd.Clear();
  if (rData.obitset.any() == false)
    return;

  cmd.set_id(rData.getTempID(m_pUser->id));
  static const set<EMemberData> setCatData = set<EMemberData>{EMEMBERDATA_BASELEVEL, EMEMBERDATA_RELIVETIME, EMEMBERDATA_EXPIRETIME, EMEMBERDATA_HP, EMEMBERDATA_MAXHP};
  for (auto &s : setCatData)
  {
    if (rData.obitset.test(s) == false)
      continue;

    SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(rData.getTempID(m_pUser->id));
    switch (s)
    {
      case EMEMBERDATA_BASELEVEL:
        GTeam::add_mdata(cmd.add_members(), s, m_pUser->getLevel());
        break;
      case EMEMBERDATA_RELIVETIME:
        GTeam::add_mdata(cmd.add_members(), s, rData.dwReliveTime);
        break;
      case EMEMBERDATA_EXPIRETIME:
        GTeam::add_mdata(cmd.add_members(), s, rData.dwExpireTime);
        break;
      case EMEMBERDATA_HP:
        GTeam::add_mdata(cmd.add_members(), s, pNpc != nullptr ? pNpc->getAttr(EATTRTYPE_HP) : rData.dwHp);
        break;
      case EMEMBERDATA_MAXHP:
        GTeam::add_mdata(cmd.add_members(), s, pNpc != nullptr ? pNpc->getAttr(EATTRTYPE_MAXHP) : 100);
        break;
      default:
        XERR << "[战斗猫-数据更新]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "队伍 :" << m_pUser->getTeamID() << "猫 :" << rData.dwID << "更新 type :" << s << "未处理" << XEND;
        break;
    }
  }
}

void WeaponPet::onUserAction(DWORD id)
{
  for (auto &s : m_oListPetData)
  {
    if (s.notIn())
      continue;
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(s.getTempID(m_pUser->id));
    if (npc == nullptr)
      continue;
    if (npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
      continue;
    npc->m_sai.checkSig("employer_action");
  }
}

void WeaponPet::onUserLevelUp()
{
  for (auto &s : m_oListPetData)
  {
    if (s.notIn())
      continue;
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(s.getTempID(m_pUser->id));
    if (npc == nullptr)
      continue;
    if (npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
      continue;
    npc->setCollectMark(ECOLLECTTYPE_BASE);
    npc->refreshDataAtonce();
  }
}

void WeaponPet::decMaxSize()
{
  if (m_dwMaxPetSize)
    m_dwMaxPetSize --;
  XLOG << "[战斗猫-数量], 最大数量减少, 玩家:" << m_pUser->name << m_pUser->id << "当前解锁数量:" << m_dwMaxPetSize << XEND;

  // 剩余时间较长的猫离队
  DWORD enablesize = 0;
  DWORD maxTime = 0;
  DWORD configid = 0;
  QWORD guid = 0;
  for (auto &s : m_oListPetData)
  {
    if (!s.bActive)
      continue;
    enablesize ++;
    if (s.dwExpireTime > maxTime)
    {
      maxTime = s.dwExpireTime;
      configid = s.dwID;
      guid = s.getTempID(m_pUser->id);
    }
  }
  if (enablesize <= m_dwMaxPetSize)
    return;
  disable(configid, false);

  KickMember cmd;
  cmd.set_userid(guid);
  cmd.set_catid(configid);
  PROTOBUF(cmd, send, len);
  thisServer->forwardCmdToSessionUser(m_pUser->id, send, len);
}

bool WeaponPet::isMyPet(QWORD id) const
{
  for (auto &s : m_oListPetData)
  {
    if (s.getTempID(m_pUser->id) == id)
      return true;
  }
  return false;
}

bool WeaponPet::checkCanActive(DWORD id) const
{
  DWORD enablesize = 0;
  for (auto &s : m_oListPetData)
  {
    if (s.bActive)
    {
      if (id == s.dwID) // 已激活时, 返回true, 表示激活成功
        return true;
      enablesize ++;
    }
  }
  return m_dwMaxPetSize > enablesize;
}

void WeaponPet::inviteHand(QWORD qwCatGUID)
{
  if (m_pUser->isHandEnable() == false)
    return;

  if (m_pUser->m_oHands.has())
    m_pUser->m_oHands.breakup();
  if (m_pUser->getHandNpc().haveHandNpc())
    m_pUser->getHandNpc().delHandNpc();
  if (m_pUser->getUserPet().handPet())
    m_pUser->getUserPet().breakHand();
  if (haveHandCat())
    breakHand();

  for (auto &s : m_oListPetData)
  {
    if (s.getTempID(m_pUser->id) != qwCatGUID)
      continue;
    if (s.notIn())
      continue;
    s.bInHandStatus = true;
    sendHandMark(qwCatGUID);
  }
}

void WeaponPet::breakHand(QWORD qwCatGUID)
{
  for (auto &s : m_oListPetData)
  {
    if (s.getTempID(m_pUser->id) != qwCatGUID)
      continue;
    if (!s.bInHandStatus)
      continue;
    s.bInHandStatus = false;
    changeState(s, EWPPETSTATAE_WAIT_BATTLE);
    sendHandBreak(qwCatGUID);
  }
}

void WeaponPet::breakHand()
{
  for (auto &s : m_oListPetData)
  {
    if (!s.bInHandStatus)
      continue;
    s.bInHandStatus = false;
    changeState(s, EWPPETSTATAE_WAIT_BATTLE);
    sendHandBreak(s.getTempID(m_pUser->id));
  }
}

void WeaponPet::leaveHand(QWORD qwCatGUID)
{
  for (auto &s : m_oListPetData)
  {
    if (s.getTempID(m_pUser->id) != qwCatGUID)
      continue;
    if (!s.bInHandStatus)
      continue;
    if (s.notIn())
      continue;
    changeState(s, EWPPETSTATAE_WAIT_BATTLE);
  }
}

void WeaponPet::sendHandStatus(bool bBuild, QWORD qwCatGUID)
{
  HandStatusUserCmd cmd;
  cmd.set_build(bBuild);
  cmd.set_masterid(m_pUser->id);
  cmd.set_followid(qwCatGUID);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToNine(send, len);
}

void WeaponPet::changeState(SWeaponPetData& stData, EWPPetState eState)
{
  if (stData.eState == eState)
    return;

  switch(stData.eState)
  {
    case EWPPETSTATAE_NORAML:
    case EWPPETSTATAE_WAIT_BATTLE:
      stData.qwFollowTeamerID = 0;
      break;
    case EWPPETSTATAE_TEAMER:
      break;
    case EWPPETSTATAE_HAND:
      sendHandStatus(false, stData.getTempID(m_pUser->id));
      break;
    default:
      break;
  }

  switch(eState)
  {
    case EWPPETSTATAE_NORAML:
    case EWPPETSTATAE_WAIT_BATTLE:
    case EWPPETSTATAE_TEAMER:
      break;
    case EWPPETSTATAE_HAND:
      sendHandStatus(true, stData.getTempID(m_pUser->id));
      break;
    default:
      break;
  }

  stData.eState = eState;
}

bool WeaponPet::haveHandCat() const
{
  for (auto &s : m_oListPetData)
  {
    if (s.bInHandStatus)
      return true;
  }

  return false;
}

void WeaponPet::sendHandBreak(QWORD qwCatGUID)
{
  BeFollowUserCmd cmd;
  cmd.set_etype(EFOLLOWTYPE_BREAK);
  cmd.set_userid(qwCatGUID);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void WeaponPet::sendHandMark(QWORD qwCatGUID)
{
  BeFollowUserCmd cmd;
  cmd.set_etype(EFOLLOWTYPE_HAND);
  cmd.set_userid(qwCatGUID);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

bool WeaponPet::isBuildHand(QWORD qwCatGUID) const
{
  for (auto &s : m_oListPetData)
  {
    if (s.getTempID(m_pUser->id) != qwCatGUID)
      continue;
    if (s.eState == EWPPETSTATAE_HAND)
      return true;
  }

  return false;
}

void WeaponPet::setExpire(DWORD id)
{
  for (auto &s : m_oListPetData)
  {
    if (s.dwID == id)
    {
      s.dwExpireTime = now();
    }
  }
}

void WeaponPet::onUserStopAction()
{
  for (auto &s : m_oListPetData)
  {
    if (s.notIn())
      continue;
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(s.getTempID(m_pUser->id));
    if (npc == nullptr)
      continue;
    if (npc->define.m_oVar.m_qwOwnerID != m_pUser->id || !npc->getAction())
      continue;
    npc->setAction(0);

    UserActionNtf cmd;
    cmd.set_type(EUSERACTIONTYPE_NORMALMOTION);
    cmd.set_value(0);
    cmd.set_charid(npc->id);
    PROTOBUF(cmd, send, len);
    npc->sendCmdToNine(send, len);
  }
}

bool WeaponPet::checkSize(DWORD id) const
{
  bool bNew = true;
  for(auto &s : m_oListPetData)
  {
    if(s.dwID == id)
    {
      bNew = false;
      break;
    }
  }

  if(bNew == true && m_oListPetData.size() >= m_dwMaxPetSize)
    return false;

  return true;
}
