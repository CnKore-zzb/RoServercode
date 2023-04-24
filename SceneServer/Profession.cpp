#include "Profession.h"
#include "UserConfig.h"
#include "SceneUser.h"
#include "Menu.h"
#include "ExchangeShop.h"

Profession::Profession(SceneUser* pUser) : m_pUser(pUser)
{
  m_dwLastLoadTime = 0;
}

Profession::~Profession()
{
}

bool Profession::load(const Cmd::BlobProfession& oData)
{
  m_mapBranch.clear();
  m_mapSvrBranch.clear();
  m_dwLastLoadTime = oData.last_load_time();
  for(int i=0; i < oData.datas_size(); ++i)
  {
    const Cmd::ProfessionData& rData = oData.datas(i);
    if(Cmd::ETypeBranch == oData.datas(i).type()) // 分支数据
    {
      Cmd::ProfessionData& data = m_mapBranch[rData.id()];
      data.CopyFrom(rData);
      Cmd::ProfessionSvrData& rSvrData = m_mapSvrBranch[rData.id()];
      rSvrData.set_id(rData.id());

      exchangeSkillcut(data);
    }
  }

  for (int i = 0; i < oData.svr_datas_size(); ++i)
  {
    const Cmd::ProfessionSvrData& rSvrData = oData.svr_datas(i);
    m_mapSvrBranch[rSvrData.id()].CopyFrom(rSvrData);
  }

  DWORD dwMaxJobLv = getMaxJobLv();
  if (m_pUser->getMaxCurJobLv() < dwMaxJobLv)
  {
    m_pUser->setMaxCurJobLv(dwMaxJobLv);
    m_pUser->checkWorldLevelBuff();
  }

  return true;
}

bool Profession::save(Cmd::BlobProfession* pData)
{
  if(!pData || !m_pUser)
    return false;

  if (m_pUser->getUserSceneData().getMainCharId() != m_pUser->id && m_pUser->getFighter() != nullptr)
  {
    DWORD dwBranch = m_pUser->getFighter()->getBranch();
    if (dwBranch != 0)
    {
      TMapExchangeShopItem mapItem;
      m_pUser->getExchangeShop().collectBranchItem(mapItem);
      if (mapItem.empty() == false)
      {
        ProfessionData& rData = m_mapBranch[dwBranch];
        for (auto &m : mapItem)
          m.second.toData(rData.add_exchange_items());
      }
    }
  }

  pData->set_last_load_time(m_dwLastLoadTime);
  for(auto& m : m_mapBranch)
  {
    Cmd::ProfessionData* data = pData->add_datas();
    if(!data)
      continue;
    m.second.clear_skillpos();
    data->CopyFrom(m.second);
  }

  pData->clear_svr_datas();
  for (auto &m : m_mapSvrBranch)
    pData->add_svr_datas()->CopyFrom(m.second);

  return true;
}

bool Profession::checkLoadTime()
{
  return now() > m_dwLastLoadTime;
}

bool Profession::sendBranchData(DWORD branch)
{
  if(!m_pUser)
    return false;

  // 因数据太大，拆分单条发送
  if(!branch)
  {
    for(auto& m : m_mapBranch)
    {
      UpdateBranchInfoUserCmd cmd;

      ProfessionUserInfo* pInfo = cmd.add_datas();
      if(!pInfo)
        continue;
      collectProfessionUserInfo(m.second, pInfo);

      PROTOBUF(cmd, send, len);
      m_pUser->sendCmdToMe(send, len);
    }
  }
  else
  {
    auto it = m_mapBranch.find(branch);
    if(m_mapBranch.end() == it)
      return false;

    UpdateBranchInfoUserCmd cmd;

    ProfessionUserInfo* pInfo = cmd.add_datas();
    if(!pInfo)
      return false;

    cmd.set_sync_type(1);
    collectProfessionUserInfo(it->second, pInfo);

    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }

  return true;
}

DWORD Profession::getBaseProfession()
{
  for(auto& m : m_mapBranch)
  {
    if(m.second.role_data().isbuy())
      continue;
    const SBranchCFG* pBranchCfg = RoleConfig::getMe().getBranchCFG(m.first);
    if(!pBranchCfg)
      continue;

    return pBranchCfg->baseId;
  }

  return 0;
}

DWORD Profession::getBuyBaseProfessionCount()
{
  DWORD dwCount = 0;
  DWORD dwBaseID = getBaseProfession();
  if(dwBaseID == 0)
  {
    const SBranchCFG* pCurBranchCfg = RoleConfig::getMe().getBranchCFG(m_pUser->getBranch());
    if(pCurBranchCfg != nullptr)
      dwBaseID = pCurBranchCfg->baseId;
  }

  for(auto& m : m_mapBranch)
  {
    if(m.second.role_data().isbuy() == false)
      continue;
    const SBranchCFG* pBranchCfg = RoleConfig::getMe().getBranchCFG(m.first);
    if(!pBranchCfg || pBranchCfg->baseId != dwBaseID)
      continue;

    ++dwCount;
  }

  return dwCount;
}

DWORD Profession::getBrotherBranchMax(const TVecDWORD& vecBrother)
{
  std::pair<DWORD, DWORD> maxBranch = std::make_pair(0,0);
  for(auto& v : vecBrother)
  {
    DWORD joblv = getJobLv(v);
    if(joblv > maxBranch.second)
      maxBranch = std::make_pair(v, joblv);
  }

  return maxBranch.first;
}

DWORD Profession::getProfessionMax()
{
  std::pair<DWORD, DWORD> maxProfession = std::make_pair(0,0);
  for(auto& m : m_mapBranch)
  {
    if(m.second.joblv() > maxProfession.second)
      maxProfession = std::make_pair(m.second.profession(), m.second.joblv());
  }

  return maxProfession.first;
}

DWORD Profession::getProfessionCount()
{
  DWORD dwCount = 0;
  for(auto& m : m_mapBranch)
  {
    if(m.second.role_data().isbuy())
      dwCount++;
  }

  return dwCount;
}

Cmd::ProfessionData* Profession::getProfessionData(DWORD dwBranch)
{
  auto m = m_mapBranch.find(dwBranch);
  if (m != m_mapBranch.end())
    return &m->second;
  return nullptr;
}

// 补偿用直接添加对应的分支和joblv
bool Profession::addBranch(DWORD dwProfession, DWORD dwJobLv)
{
  if(!m_pUser || !m_pUser->getFighter())
    return false;

  if(m_pUser->getEvo() < 2)
    return false;

  const SRoleBaseCFG* pRoleCfg = RoleConfig::getMe().getRoleBase(static_cast<EProfession>(dwProfession));
  if(!pRoleCfg)
    return false;

  DWORD dwBranch = pRoleCfg->dwTypeBranch;
  if(hasBranch(dwBranch))
  {
    XERR << "[多职业-添加职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "has same branch! branch:" << dwBranch << XEND;
    return false;
  }

  const SBranchCFG* pBranchCfg = RoleConfig::getMe().getBranchCFG(dwBranch);
  if(!pBranchCfg)
  {
    XERR << "[多职业-添加职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get branch config failed! profession:" << dwProfession << "joblv:" << dwJobLv << XEND;
    return false;
  }

  // 复刻职业最高三转
  if(PEAK_PRO_MAX_JOBLV < dwJobLv && pBranchCfg->professionPeak)
    dwProfession = pBranchCfg->professionPeak;

  // 复刻职业等级不能高于主号最大职业等级
  DWORD dwMaxJoblv = m_pUser->getFighter()->getMaxJobLv();
  if(PEAK_PRO_MAX_JOBLV < dwMaxJoblv)
    dwMaxJoblv = PEAK_PRO_MAX_JOBLV;
  else if(THREE_GRADE_PRO_MAX_JOBLV > dwMaxJoblv)
    dwMaxJoblv = THREE_GRADE_PRO_MAX_JOBLV;

  if(dwJobLv > dwMaxJoblv)
    dwJobLv = dwMaxJoblv;

  DWORD dwTime = now();
  Cmd::ProfessionData& data = m_mapBranch[dwBranch];
  data.set_id(dwBranch);
  data.set_profession(dwProfession);
  data.set_joblv(dwJobLv);
  data.set_type(Cmd::ETypeBranch);
  data.set_isfirst(true);
  data.set_recordtime(dwTime);

  if(!initRoleData(dwBranch, dwProfession, dwJobLv))
  {
    // 添加初始化失败
    m_mapBranch.erase(dwBranch);
    XERR << "[多职业-添加职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "init role data failed! profession:" << dwProfession << "joblv:" << dwJobLv << XEND;
    return false;
  }

  if(dwJobLv > ONE_GRADE_PRO_MAX_JOBLV)
  {
    // 添加一转任务至任务完成列表
    for(auto& q : pBranchCfg->vecDelTask)
      m_pUser->getQuest().finishQuest(q);
  }

  if(dwJobLv > THREE_GRADE_PRO_MAX_JOBLV)
    m_pUser->getMenu().open(EMENUID_PEAK_LEVEL);

  Cmd::ProfessionSvrData& rSvrData = m_mapSvrBranch[dwBranch];
  rSvrData.set_id(dwBranch);
  rSvrData.set_bepro_1_time(dwTime);

  XLOG << "[多职业-添加职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "add branch success! branch:" << dwBranch << XEND;
  return true;
}

bool Profession::addBranch(DWORD dwBranch)
{
  // 检测表中是否存在该分支；
  // 从表中获取该分支的基础职业、相邻分支；
  // 设置职业=基础职业；
  // 如果相邻分支存在，获取相邻分支的职业、等级；
  // 如果职业不等于基础职业，设置joblv=50（class表职业最大等级）；如果职业等于基础职业，设置joblv=该分支joblv；
  // 如果相邻分支不存在，设置joblv=11（class表初心者最大等级+1）；
  if(!m_pUser)
    return false;

  if(hasBranch(dwBranch))
  {
    XERR << "[多职业-添加职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "has same branch! branch:" << dwBranch << XEND;
    return false;
  }

  const SBranchCFG* pBranchCfg = RoleConfig::getMe().getBranchCFG(dwBranch);
  if(!pBranchCfg || !pBranchCfg->baseId)
  {
    XERR << "[多职业-添加职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get SBranchCFG failed! branch:" << dwBranch << XEND;
    return false;
  }

  DWORD dwCurBranch = m_pUser->getBranch();
  DWORD dwProfessionBase = pBranchCfg->baseId;
  DWORD dwBrotherBranch = getBrotherBranchMax(pBranchCfg->vecBrotherId);
  const SBranchCFG* pCurBranchCfg = RoleConfig::getMe().getBranchCFG(dwCurBranch);
  if(!pCurBranchCfg)
  {
    XERR << "[多职业-添加职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get SBranchCFG failed! branch:" << dwCurBranch << XEND;
    return false;
  }

  DWORD dwJobLv = 0;
  if(pCurBranchCfg->baseId == dwProfessionBase && m_pUser->getJobLv() > getJobLv(dwBrotherBranch))
  {
    dwJobLv = m_pUser->getJobLv();
  }
  else
  {
    dwJobLv = getJobLv(dwBrotherBranch);
  }

  const SRoleBaseCFG* pRoleNovCfg = RoleConfig::getMe().getRoleBase(EPROFESSION_NOVICE);
  const SRoleBaseCFG* pRoleBaseCfg = RoleConfig::getMe().getRoleBase(static_cast<Cmd::EProfession>(dwProfessionBase));
  if(!pRoleNovCfg || !pRoleBaseCfg)
  {
    XERR << "[多职业-添加职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get SRoleBaseCFG failed! novice:" << EPROFESSION_NOVICE << "profession:" << dwProfessionBase << XEND;
    return false;
  }

  if(dwJobLv >= pRoleBaseCfg->maxJobLv)
  {
    dwJobLv = pRoleBaseCfg->maxJobLv;
  }
  else if(dwJobLv <= pRoleNovCfg->maxJobLv)
  {
    dwJobLv = pRoleNovCfg->maxJobLv + 1;
  }

  DWORD dwNow = now();
  Cmd::ProfessionData& data = m_mapBranch[dwBranch];
  data.set_id(dwBranch);
  data.set_profession(dwProfessionBase);
  data.set_joblv(dwJobLv);
  data.set_type(Cmd::ETypeBranch);
  data.set_isfirst(true);
  data.set_recordtime(now());

  if(!initRoleData(dwBranch, dwProfessionBase, dwJobLv))
  {
    // 添加初始化失败
    m_mapBranch.erase(dwBranch);
    XERR << "[多职业-添加职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "init role data failed! profession:" << dwProfessionBase << "joblv:" << dwJobLv << XEND;
    return false;
  }

  Cmd::ProfessionSvrData& rSvrData = m_mapSvrBranch[dwBranch];
  rSvrData.set_id(dwBranch);
  rSvrData.set_bepro_1_time(dwNow);

  m_pUser->getEvent().onProfessionAdd();
  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_BUY_PROFESSION);
  XLOG << "[多职业-添加职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "add branch success! branch:" << dwBranch << XEND;
  return true;
}

bool Profession::initRoleData(DWORD dwBranch, DWORD dwProfession, DWORD dwJobLv)
{
  if(!m_pUser)
    return false;

  if(!hasBranch(dwBranch))
    return false;

  const SRoleBaseCFG* pRoleNovCfg = RoleConfig::getMe().getRoleBase(EPROFESSION_NOVICE);
  const SRoleBaseCFG* pRoleBaseCfg = RoleConfig::getMe().getRoleBase(static_cast<Cmd::EProfession>(dwProfession));
  if(!pRoleNovCfg || !pRoleBaseCfg)
  {
    XERR << "[多职业-添加职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get SRoleBaseCFG failed! novice:" << EPROFESSION_NOVICE << "profession:" << dwProfession << XEND;
    return false;
  }

  Cmd::ProfessionData& data = m_mapBranch[dwBranch];
  UserRoleData* pRoleData = data.mutable_role_data();
  if(!pRoleData)
  {
    XERR << "[多职业-添加职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get UserRoleData failed!" << XEND;
    return false;
  }

  pRoleData->Clear();
  pRoleData->set_branch(dwBranch);
  pRoleData->set_joblv(dwJobLv);
  pRoleData->set_profession(static_cast<EProfession>(dwProfession));
  pRoleData->set_totalpoint(m_pUser->getTotalPoint());
  pRoleData->set_isbuy(true);

  // skill
  UserSkillData* pSkillData = pRoleData->mutable_skill();
  if(!pSkillData)
  {
    XERR << "[多职业-添加职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get UserSkillData failed!" << XEND;
    return false;
  }

  pSkillData->set_totalpoint(dwJobLv - pRoleNovCfg->maxJobLv);
  pSkillData->set_maxpos(m_pUser->getMaxSkillPos());
  pSkillData->set_automaxpos(m_pUser->getAutoSkillPos());
  pSkillData->set_maxextendpos(m_pUser->getExtendSkillPos());
  pSkillData->set_reseted(true);

  return true;
}

// 检测是否已购买该分支
bool Profession::hasBranch(DWORD dwBranch)
{
  return m_mapBranch.end() != m_mapBranch.find(dwBranch);
}

// 获取该分支当前等级
DWORD Profession::getJobLv(DWORD dwBranch)
{
  auto item = m_mapBranch.find(dwBranch);
  if(m_mapBranch.end() == item)
    return 0;

  return item->second.joblv();
}

//获取所有分支的最大JobLv
DWORD Profession::getMaxJobLv()
{
  DWORD dwMaxJobLv = 0;
  for (auto& it : m_mapBranch)
  {
    if (it.second.joblv() > dwMaxJobLv)
    {
      dwMaxJobLv = it.second.joblv();
    }
  }
  return dwMaxJobLv;
}

// 切换职业同步joblv
void Profession::setJobLv(DWORD dwBranch, DWORD dwJoblv)
{
  if(!m_pUser)
    return;

  auto item = m_mapBranch.find(dwBranch);
  if(m_mapBranch.end() == item)
    return;

  DWORD dwOldJoblv = item->second.joblv();
  if(dwJoblv <= dwOldJoblv)
    return;

  item->second.set_joblv(dwJoblv);

  // role
  UserRoleData* pRoleData = item->second.mutable_role_data();
  if(!pRoleData)
  {
    XERR << "[多职业-设置分支joblv]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get UserRoleData failed!" << XEND;
    return;
  }

  pRoleData->set_joblv(dwJoblv);

  // skill
  UserSkillData* pSkillData = pRoleData->mutable_skill();
  if(!pSkillData)
  {
    XERR << "[多职业-设置分支joblv]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get UserSkillData failed!" << XEND;
    return;
  }

  DWORD dwAddPoint = dwJoblv - dwOldJoblv;
  pSkillData->set_totalpoint(pSkillData->totalpoint() + dwAddPoint);

  sendBranchData(dwBranch);
  XDBG << "[多职业-设置分支joblv]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "success! oldJoblv:" << dwOldJoblv << "newJoblv:" << dwJoblv << XEND;
}

//获取该分支的技能点总数
DWORD Profession::getTotalSkillPoint(DWORD dwBranch)
{
  if (dwBranch == m_pUser->getBranch())
  {
    const FighterSkill& oSkill = m_pUser->getFighter()->getSkill();
    return oSkill.getSkillPoint() + oSkill.getUsedPoint();
  }
  auto item = m_mapBranch.find(dwBranch);
  if(m_mapBranch.end() == item)
    return 0;

  const UserSkillData& skill = item->second.role_data().skill();
  DWORD dwTotalPoint = skill.totalpoint();
  for (int i = 0; i < skill.datas_size(); ++i)
  {
    dwTotalPoint += skill.datas(i).usedpoint();
  }

  return dwTotalPoint;
}

// 请求已购买职业分支
void Profession::queryBranchs(Cmd::ProfessionQueryUserCmd& cmd)
{
  if(!m_pUser)
    return;

  for(auto& m : m_mapBranch)
  {
    if(BRANCH_BACKUP == m.first)
      continue;

    Cmd::ProfessionInfo* pData = cmd.add_items();
    if(!pData)
      continue;

    if(m.second.role_data().branch() == m_pUser->getBranch())
    {
      pData->set_branch(m_pUser->getBranch());
      pData->set_profession(m_pUser->getProfession());
      pData->set_joblv(m_pUser->getJobLv());
      pData->set_iscurrent(true);
    }
    else
    {
      pData->set_branch(m.second.id());
      pData->set_profession(m.second.profession());
      pData->set_joblv(m.second.joblv());
    }

    pData->set_isbuy(m.second.role_data().isbuy());
  }
}

// 加载职业数据
bool Profession::loadProfessionData(Cmd::ProfessionData& data, bool isFirst)
{
  if(!m_pUser)
    return false;

  // 断开牵手
  m_pUser->getUserPet().breakHand();

  std::vector<std::pair<DWORD, DWORD>> vecStars; // 星位列表
  if(!resetAstrolabesData(vecStars))
  {
    XERR << "[多职业-切换职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "reset astrolabes data failed!" << XEND;
    return false;
  }

  bool isReset = false;
  if(!loadRoleData(data.role_data(), isFirst))
  {
    XERR << "[多职业-切换职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "load role data failed!" << XEND;
    return false;
  }

  if(!loadAstrolabesData(data.astrolabe_data(), vecStars, isReset, isFirst))
  {
    XERR << "[多职业-切换职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "load astrolabes data failed!" << XEND;
    return false;
  }

  if(!loadBeingData(data.being_data()))
  {
    XERR << "[多职业-切换职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "load Being data failed!" << XEND;
    return false;
  }

  if(!loadPackageData(data, isFirst))
  {
    XERR << "[多职业-切换职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "load package data failed!" << XEND;
    return false;
  }

  if (!((m_pUser->getProfession() >= EPROFESSION_MERCHANT && m_pUser->getProfession() <= EPROFESSION_MECHANIC) || (m_pUser->getProfession() >= EPROFESSION_ALCHEMIST && m_pUser->getProfession() <= EPROFESSION_GENETIC)))
  {
    //不是商人系职业 才加载partner 因为手推车信息，会在加载装备的时候添加
    if(!loadPartnerData(data.partner_data()))
    {
      XERR << "[多职业-切换职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "load partner data failed!" << XEND;
      return false;
    }
  }

  if (!loadExchangeShopData(data))
  {
    XERR << "[多职业-切换职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "load exchange shop data failed!" << XEND;
    return false;
  }

  // 星盘影响生命体技能点数、buff、技能
  // 如果星盘重置，生命体的额外技能点数、额外buff、技能改变 都要重置掉
  if(isReset)
  {
    m_pUser->getUserBeing().onRuneReset();
  }

  m_pUser->setCollectMark(ECOLLECTTYPE_BASE);
  //m_pUser->setCollectMark(ECOLLECTTYPE_SKILL);
  m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
  //m_pUser->setCollectMark(ECOLLECTTYPE_CARD);
  m_pUser->setCollectMarkAllBuff();
  m_pUser->setCollectMark(ECOLLECTTYPE_ASTROLABE);

  m_pUser->updateAttribute();

  m_pUser->getFighter()->setHp(m_pUser->getAttr(EATTRTYPE_MAXHP));
  m_pUser->sendFighterInfo();
  m_pUser->getEvent().onSkillPointChange();

  m_pUser->setDataMark(EUSERDATATYPE_BODY);
  m_pUser->setDataMark(EUSERDATATYPE_LEFTHAND);
  m_pUser->setDataMark(EUSERDATATYPE_RIGHTHAND);
  m_pUser->setDataMark(EUSERDATATYPE_PET_PARTNER);
  m_pUser->setDataMark(EUSERDATATYPE_STRPOINT);
  m_pUser->setDataMark(EUSERDATATYPE_INTPOINT);
  m_pUser->setDataMark(EUSERDATATYPE_AGIPOINT);
  m_pUser->setDataMark(EUSERDATATYPE_DEXPOINT);
  m_pUser->setDataMark(EUSERDATATYPE_VITPOINT);
  m_pUser->setDataMark(EUSERDATATYPE_LUKPOINT);
  m_pUser->setDataMark(EUSERDATATYPE_TOTALPOINT);
  m_pUser->setDataMark(EUSERDATATYPE_NORMAL_SKILL);
  m_pUser->setDataMark(EUSERDATATYPE_JOBLEVEL);
  m_pUser->setDataMark(EUSERDATATYPE_JOBEXP);
  m_pUser->setDataMark(EUSERDATATYPE_SKILL_POINT);

  // 同步
  m_pUser->syncProfessionData(data.type());
  return true;
}

// 保存职业数据
bool Profession::saveProfessionData(Cmd::ProfessionData& data)
{
  if(!m_pUser || !m_pUser->getFighter())
    return false;

  //属性预览信息
  UserAttribute* pAttr = dynamic_cast<UserAttribute*>(m_pUser->getAttribute());
  if(pAttr)
    pAttr->toPreviewBlobAttr(data.mutable_attr_data());

  //冒险技能预览信息
  SceneFighter* pNoviceFighter = m_pUser->getFighter(EPROFESSION_NOVICE);
  if (pNoviceFighter != nullptr)
  {
    SSkillData* pData = pNoviceFighter->getSkill().getSkillData(EPROFESSION_NOVICE);
    if (pData == nullptr)
      return false;
    SkillData* pNoviceSkill = data.mutable_novice_data();
    if (pNoviceSkill == nullptr)
      return false;
    pData->toClientData(pNoviceSkill, m_pUser->getFighter());
  }
  
  //技能快捷栏预览信息
  SceneFighter* pFighter = m_pUser->getCurFighter();
  if (pFighter != nullptr)
  {
    /*SkillValidPosData* pSkillPos = data.mutable_skillpos();
    if (pSkillPos == nullptr)
      return false;*/
    pFighter->getSkill().toData(data.mutable_shortcut());
  }

  //角色外观预览信息
  m_pUser->getUserSceneData().toPreviewAppearanceData(data);

  data.set_profession(m_pUser->getProfession());
  data.set_joblv(m_pUser->getJobLv());
  data.set_jobexp(m_pUser->getFighter()->getJobExp());
  data.set_isfirst(false);

  if(!saveRoleData(data.mutable_role_data()))
  {
    XERR << "[多职业-切换职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "save role data failed!" << XEND;
    return false;
  }

  if(!saveBeingData(data.mutable_being_data()))
  {
    XERR << "[多职业-切换职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "save being data failed!" << XEND;
    return false;
  }

  if(!saveAstrolabesData(data.mutable_astrolabe_data()))
  {
    XERR << "[多职业-切换职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "save astrolabes data failed!" << XEND;
    return false;
  }

  if(!savePackageData(data))
  {
    XERR << "[多职业-切换职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "save package data failed!" << XEND;
    return false;
  }
  if(!savePartnerData(data.mutable_partner_data()))
  {
    XERR << "[多职业-切换职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "save partner data failed!" << XEND;
    return false;
  }
  if (!saveExchangeShopData(data))
  {
    XERR << "[多职业-切换职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "save exchangeshop data failed!" << XEND;
    return false;
  }

  XLOG << "[多职业-切换职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "save professiondata success! joblv:" << m_pUser->getJobLv() << "jobexp:" << m_pUser->getFighter()->getJobExp() << XEND;
  return true;
}

bool Profession::saveCurProfessionData()
{
  if(!m_pUser)
    return false;

  DWORD dwBranchCur = m_pUser->getBranch();
  if(!hasBranch(dwBranchCur))
  {
    // 第一次切换分支
    Cmd::ProfessionData& data_first = m_mapBranch[dwBranchCur];
    data_first.set_id(dwBranchCur);
    data_first.set_type(Cmd::ETypeBranch);
  }

  Cmd::ProfessionData& data_cur = m_mapBranch[dwBranchCur];

  return saveProfessionData(data_cur);
}

bool Profession::loadRecordProfessionData(Cmd::ProfessionData& record_data)
{
  DWORD dwBranchDest = record_data.pro_branch();
  if (!hasBranch(dwBranchDest))
  {
    // log
    XERR << "[角色存档-加载存档]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "has not branch! branch:" << dwBranchDest << XEND;
    return false;
  }
  Cmd::ProfessionData& branch_data = m_mapBranch[dwBranchDest];
  Cmd::ProfessionData tmp_data;
  tmp_data.CopyFrom(branch_data);
  tmp_data.set_type(record_data.type());
  tmp_data.set_pro_branch(record_data.pro_branch());

  //迁移装备信息
  tmp_data.clear_pack_data();
  for (int i = 0; i < record_data.pack_data_size(); ++i)
  {
    EquipPackData* pPackData = tmp_data.add_pack_data();
    if (pPackData == nullptr)
      return false;
    pPackData->CopyFrom(record_data.pack_data(i));
  }

  //迁移属性加点
  UserRoleData* pRoleData = tmp_data.mutable_role_data();
  const UserRoleData& recordRoleData = record_data.role_data();
  pRoleData->set_strpoint(recordRoleData.strpoint());
  pRoleData->set_intpoint(recordRoleData.intpoint());
  pRoleData->set_agipoint(recordRoleData.agipoint());
  pRoleData->set_dexpoint(recordRoleData.dexpoint());
  pRoleData->set_vitpoint(recordRoleData.vitpoint());
  pRoleData->set_lukpoint(recordRoleData.lukpoint());
  DWORD dwTotalPoint = m_pUser->getTotalPoint();
  DWORD dwLeftPoint = dwTotalPoint > recordRoleData.usedpoint() ? (dwTotalPoint - recordRoleData.usedpoint()) : 0;
  pRoleData->set_totalpoint(dwLeftPoint);
  pRoleData->set_usedpoint(recordRoleData.usedpoint());

  //迁移职业技能信息
  UserSkillData* pSkill = pRoleData->mutable_skill();
  if (pSkill == nullptr)
    return false;
  const UserSkillData& skill = recordRoleData.skill();
  DWORD dwTotalSkillPoint = pSkill->totalpoint();
  DWORD dwRecordUsedPoint = 0;
  DWORD dwLeftSkillPoint = 0;
  for (int i = 0; i < pSkill->datas_size(); ++i)
  {
    dwTotalSkillPoint += pSkill->datas(i).usedpoint();
  }

  pSkill->clear_datas();
  for (int i = 0; i < skill.datas_size(); ++i)
  {
    SkillData* pSkillData = pSkill->add_datas();
    if (pSkillData == nullptr)
      return false;
    dwRecordUsedPoint += skill.datas(i).usedpoint();
    pSkillData->CopyFrom(skill.datas(i));
  }

  dwLeftSkillPoint = dwTotalSkillPoint > dwRecordUsedPoint ? (dwTotalSkillPoint - dwRecordUsedPoint) : 0;
  pSkill->set_totalpoint(dwLeftSkillPoint);

  // 快捷栏
  map<ESkillShortcut, map<DWORD, SkillPos>> mapShortcut;
  for (int i = 0; i < skill.shortcuts_size(); ++i)
  {
    const SkillShortcutDB& rCut = skill.shortcuts(i);
    map<DWORD, SkillPos>& mapPos = mapShortcut[rCut.type()];
    for (int j = 0; j < rCut.cuts_size(); ++j)
    {
      const SkillPos& rPos = rCut.cuts(j);
      mapPos[rPos.id()].CopyFrom(rPos);
    }
  }

  pSkill->clear_pos();
  map<DWORD, SkillPos>& mapPos = mapShortcut[ESKILLSHORTCUT_NORMAL];
  for (int i = 0; i < skill.pos_size(); ++i)
  {
    const SkillPos& rPos = skill.pos(i);
    mapPos[rPos.id()].CopyFrom(rPos);
  }

  pSkill->clear_autopos();
  map<DWORD, SkillPos>& mapAutoPos = mapShortcut[ESKILLSHORTCUT_AUTO];
  for (int i = 0; i < skill.autopos_size(); ++i)
  {
    const SkillPos& rPos = skill.autopos(i);
    mapAutoPos[rPos.id()].CopyFrom(rPos);
  }

  pSkill->clear_extendpos();
  map<DWORD, SkillPos>& mapExtendPos = mapShortcut[ESKILLSHORTCUT_EXTEND];
  for (int i = 0; i < skill.extendpos_size(); ++i)
  {
    const SkillPos& rPos = skill.extendpos(i);
    mapExtendPos[rPos.id()].CopyFrom(rPos);
  }

  pSkill->clear_shortcuts();
  for (auto &m : mapShortcut)
  {
    SkillShortcutDB* pCut = pSkill->add_shortcuts();
    pCut->set_type(m.first);
    for (auto &p : m.second)
      pCut->add_cuts()->CopyFrom(p.second);
  }

  pSkill->clear_replace();
  for (int i = 0; i < skill.replace_size(); ++i)
  {
    SkillReplaceInfo* pSkillReplace = pSkill->add_replace();
    if (pSkillReplace == nullptr)
      return false;
    pSkillReplace->CopyFrom(skill.replace(i));
  }

  //迁移生命体技能信息
  std::map<DWORD, bool> mapBeingHasSameSkill;
  BlobUserBeing* pBeing = tmp_data.mutable_being_data();
  if (pBeing == nullptr)
    return false;
  const BlobUserBeing& recordBeing = record_data.being_data();
  pBeing->set_curbeingid(recordBeing.curbeingid());
  pBeing->set_skillpoint(recordBeing.skillpoint());
  for (int i = 0; i < recordBeing.data_size(); ++i)
  {
    const UserBeingData& recordBeingData = recordBeing.data(i);
    for (int j = 0; j < pBeing->data_size(); ++j)
    {
      UserBeingData* pBeingData = pBeing->mutable_data(j);
      if (pBeingData == nullptr)
        return false;
      if (pBeingData->id() != recordBeingData.id())
        continue;
      pBeingData->set_usedskillpoint(recordBeingData.usedskillpoint());
      TSetDWORD setSkillIds;
      //这里不能简单的清空后 拷贝，要把存档里没有的技能保留下来
      for (int k = 0; k < recordBeingData.skills_size(); ++k)
      {
        if (setSkillIds.count(recordBeingData.skills(k).id()/1000) > 0)
        {
          mapBeingHasSameSkill[recordBeingData.id()] = true;
        }
        setSkillIds.insert(recordBeingData.skills(k).id()/1000);

        bool bSameSkill = false;
        for (int m = 0; m < pBeingData->skills_size(); ++m)
        {
          SkillItem* pSkill = pBeingData->mutable_skills(m);
          if (pSkill == nullptr)
            return false;
          if (pSkill->id()/1000 == recordBeingData.skills(k).id()/1000)
          {
            bSameSkill = true;
            pSkill->CopyFrom(recordBeingData.skills(k));
            break;
          }
        }
        if (!bSameSkill)
        {
          SkillItem* pSkill = pBeingData->add_skills();
          if (pSkill == nullptr)
            return false;
          pSkill->CopyFrom(recordBeingData.skills(k));
        }
      }
    }
  }

  //迁移星盘信息
  BlobAstrolabe* pAstrolabe = tmp_data.mutable_astrolabe_data();
  if (pAstrolabe == nullptr)
    return false;
  pAstrolabe->Clear();
  pAstrolabe->CopyFrom(record_data.astrolabe_data());

  //迁移partner信息
  BlobPet* pPet = tmp_data.mutable_partner_data();
  if (pPet == nullptr)
    return false;
  pPet->set_activepartner(record_data.partner_data().activepartner());

  bool ret = loadProfessionData(tmp_data);

  if (ret)
  {
    for (auto it : mapBeingHasSameSkill)
    {
      if (it.second)
        m_pUser->getUserBeing().resetAllSkill(it.first);
    }
  }
  return ret;
}

bool Profession::collectProfessionUserInfo(const Cmd::ProfessionData& src_data, Cmd::ProfessionUserInfo* dest_data)
{
  if (dest_data == nullptr)
    return false;

  dest_data->set_id(src_data.id());
  dest_data->set_profession(src_data.profession());
  dest_data->set_joblv(src_data.joblv());
  dest_data->set_jobexp(src_data.jobexp());
  dest_data->set_type(src_data.type());
  dest_data->set_recordname(src_data.recordname());
  dest_data->set_recordtime(src_data.recordtime());
  dest_data->set_charid(src_data.charid());
  dest_data->set_charname(src_data.charname());
  dest_data->set_isfirst(src_data.isfirst());

  //预览属性
  AttrProfessionData* dest_attr = dest_data->mutable_attr_data();
  if (dest_attr == nullptr)
    return false;
  for (int i = 0; i < src_data.attr_data().datas_size(); i++)
  {
    UserAttr* attr = dest_attr->add_attrs();
    if (attr == nullptr)
      continue;
    attr->set_type(src_data.attr_data().datas(i).type());
    attr->set_value((int)src_data.attr_data().datas(i).value());
  }

  //预览角色外观
  for (int i = 0; i < src_data.appearance_data_size(); i++)
  {
    UserData* user_data = dest_attr->add_datas();
    if (user_data == nullptr)
      continue;
    user_data->CopyFrom(src_data.appearance_data(i));
  }

  //预览装备
  for (int i = 0; i < src_data.pack_data_size(); i++)
  {
    EquipPackData* equip_pack = dest_data->add_equip_data();
    if (equip_pack == nullptr)
      continue;
    equip_pack->CopyFrom(src_data.pack_data(i));
  }

  //预览星盘
  AstrolabeProfessionData* dest_astrolabe = dest_data->mutable_astrolabe_data();
  if (dest_astrolabe == nullptr)
    return false;
  const BlobAstrolabe& src_astrolabe = src_data.astrolabe_data();
  for (int i = 0; i < src_astrolabe.datas_size(); ++i)
  {
    const AstrolabeMainData& main_data = src_astrolabe.datas(i);
    for (int j = 0; j < main_data.astrolabes_size(); ++j)
    {
      const AstrolabeData& astrolabe_data = main_data.astrolabes(j);
      for (int k = 0; k < astrolabe_data.stars_size(); ++k)
      {
        const AstrolabeStarData& star_data = astrolabe_data.stars(k);
        dest_astrolabe->add_stars(astrolabe_data.id() * 10000 + star_data.id());
      }
    }
  }

  //预览技能
  SkillProfessionData* dest_skill = dest_data->mutable_skill_data();
  if (dest_skill == nullptr)
    return false;

  dest_skill->mutable_shortcut()->CopyFrom(src_data.shortcut());

  const SkillValidPosData& rData = src_data.skillpos();
  SkillValidPos* pData = dest_skill->mutable_shortcut();
  if (rData.pos_size() != 0)
  {
    SkillValidShortcut* pCut = pData->add_shortcuts();
    pCut->set_type(ESKILLSHORTCUT_NORMAL);
    for (int i = 0; i < rData.pos_size(); ++i)
      pCut->add_pos(rData.pos(i));
  }
  if (rData.autopos_size() != 0)
  {
    SkillValidShortcut* pCut = pData->add_shortcuts();
    pCut->set_type(ESKILLSHORTCUT_AUTO);
    for (int i = 0; i < rData.autopos_size(); ++i)
      pCut->add_pos(rData.autopos(i));
  }
  if (rData.extendpos_size() != 0)
  {
    SkillValidShortcut* pCut = pData->add_shortcuts();
    pCut->set_type(ESKILLSHORTCUT_EXTEND);
    for (int i = 0; i < rData.extendpos_size(); ++i)
      pCut->add_pos(rData.extendpos(i));
  }

  SkillData* pNoviceSkill =  dest_skill->add_datas();
  if (pNoviceSkill)
  {
    pNoviceSkill->CopyFrom(src_data.novice_data());

    for (int i = 0; i < pNoviceSkill->items_size(); ++i)
    {
      SkillItem* pItem = pNoviceSkill->mutable_items(i);

      if (pItem->pos() != 0)
      {
        SkillShortcut* pCut = pItem->add_shortcuts();
        pCut->set_type(ESKILLSHORTCUT_NORMAL);
        pCut->set_pos(pItem->pos());
      }
      if (pItem->autopos() != 0)
      {
        SkillShortcut* pCut = pItem->add_shortcuts();
        pCut->set_type(ESKILLSHORTCUT_AUTO);
        pCut->set_pos(pItem->autopos());
      }
      if (pItem->extendpos() != 0)
      {
        SkillShortcut* pCut = pItem->add_shortcuts();
        pCut->set_type(ESKILLSHORTCUT_EXTEND);
        pCut->set_pos(pItem->autopos());
      }
    }
  }

  DWORD dwTotalSkillPoint = 0;
  DWORD dwUsedSkillPoint = 0;
  DWORD dwBranchID = src_data.id();
  if (src_data.type() == ETypeRecord)
  {
    dwBranchID = src_data.pro_branch();
  }
  const UserSkillData& src_skill = src_data.role_data().skill();
  for (int i = 0; i < src_skill.datas_size(); ++i)
  {
    if (src_skill.datas(i).profession() == EPROFESSION_NOVICE)
      continue;
    dwUsedSkillPoint += src_skill.datas(i).usedpoint();

    SkillData* pDestSkill = dest_skill->add_datas();
    if(pDestSkill)
    {
      pDestSkill->CopyFrom(src_skill.datas(i));

      for (int i = 0; i < pDestSkill->items_size(); ++i)
      {
        SkillItem* pItem = pDestSkill->mutable_items(i);
        if (pItem->pos() != 0)
        {
          SkillShortcut* pCut = pItem->add_shortcuts();
          pCut->set_type(ESKILLSHORTCUT_NORMAL);
          pCut->set_pos(pItem->pos());
        }
        if (pItem->autopos() != 0)
        {
          SkillShortcut* pCut = pItem->add_shortcuts();
          pCut->set_type(ESKILLSHORTCUT_AUTO);
          pCut->set_pos(pItem->autopos());
        }
        if (pItem->extendpos() != 0)
        {
          SkillShortcut* pCut = pItem->add_shortcuts();
          pCut->set_type(ESKILLSHORTCUT_EXTEND);
          pCut->set_pos(pItem->autopos());
        }
      }
    }
  }
  //不是本角色的存档利用存档里面的数据计算
  if (src_data.type() == ETypeRecord && src_data.charid() != m_pUser->id)
  {
    dwTotalSkillPoint = src_skill.totalpoint() + dwUsedSkillPoint;
  }
  else
  {
    dwTotalSkillPoint = getTotalSkillPoint(dwBranchID);
  }
  dest_skill->set_left_point(dwTotalSkillPoint > dwUsedSkillPoint ? (dwTotalSkillPoint - dwUsedSkillPoint) : 0);

  auto calcTotalPoint = [&](DWORD dwPoint, DWORD dwLv) -> DWORD
  {
    DWORD v = 0;
    v += dwPoint;
    const SBeingBaseLvCFG* cfg = BeingConfig::getMe().getBeingBaseLvCFG(dwLv);
    if (cfg != nullptr)
      v += cfg->dwSkillPoint;
    return v;
  };

  const BlobUserBeing& src_being = src_data.being_data();
  dest_skill->set_curbeingid(src_being.curbeingid());
  for (int i = 0; i < src_being.data_size(); ++i)
  {
    const UserBeingData& being_data = src_being.data(i);

    //生命体信息
    BeingInfo* pDestBeingInfo = dest_skill->add_beinginfos();
    if(pDestBeingInfo == nullptr)
      continue;
    pDestBeingInfo->set_beingid(being_data.id());
    pDestBeingInfo->set_exp(being_data.exp());
    pDestBeingInfo->set_lv(being_data.lv());
    pDestBeingInfo->set_battle(being_data.battle());
    pDestBeingInfo->set_live(being_data.live());
    pDestBeingInfo->set_body(being_data.body());
    for (int j = 0; j < being_data.bodylist_size(); ++j)
    {
      pDestBeingInfo->add_bodylist(being_data.bodylist(j));
    }

    //生命体技能信息
    BeingSkillData* pDestBeingData = dest_skill->add_beings();
    if(pDestBeingData == nullptr)
      continue;
    pDestBeingData->set_id(being_data.id());
    pDestBeingData->set_usedpoint(being_data.usedskillpoint());
    DWORD total_point = calcTotalPoint(src_being.skillpoint(), being_data.lv());
    pDestBeingData->set_leftpoint(total_point >= being_data.usedskillpoint() ? (total_point - being_data.usedskillpoint()) : 0);
    for (int j = 0; j < being_data.skills_size(); ++j)
    {
      SkillItem* pSkillItem = pDestBeingData->add_items();
      if(pSkillItem)
      {
        pSkillItem->CopyFrom(being_data.skills(j));
      }
    }
  }
  return true;
}

// 重置星盘数据
// 星盘与技能耦合度较高，需要在加载之前重置
bool Profession::resetAstrolabesData(std::vector<std::pair<DWORD, DWORD>>& vecStars)
{
  if(!m_pUser)
    return false;

  return m_pUser->getAstrolabes().resetAll(vecStars);
}

// 加载星盘数据
bool Profession::loadAstrolabesData(const Cmd::BlobAstrolabe& data, const std::vector<std::pair<DWORD, DWORD>>& vecStars, bool& isReset, bool isFirst)
{
  if(!m_pUser)
    return false;

  return m_pUser->getAstrolabes().loadProfessionData(data, vecStars, isReset, isFirst);
}

// 保存星盘数据
bool Profession::saveAstrolabesData(Cmd::BlobAstrolabe* pData)
{
  if(!m_pUser || !pData)
    return false;

  return m_pUser->getAstrolabes().save(pData);
}

// 加载fighter数据
bool Profession::loadRoleData(const Cmd::UserRoleData& data, bool isFirst)
{
  if(!m_pUser || !m_pUser->getFighter())
    return false;

  return m_pUser->getFighter()->loadProfessionData(data, isFirst);
}

// 保存fighter数据
bool Profession::saveRoleData(Cmd::UserRoleData* pData)
{
  if(!m_pUser || !m_pUser->getFighter())
    return false;

  return m_pUser->getFighter()->saveProfessionData(pData);
}

// 加载装备数据
bool Profession::loadPackageData(const Cmd::ProfessionData& data, bool isFirst)
{
  if(!m_pUser)
    return false;

  // 第一次切换至目标职业
  // 装备不需要穿上（其实是没得换）
  bool isNeedPutOn = isFirst ? false : true;
  return m_pUser->getPackage().loadProfessionData(data, isNeedPutOn);
}

// 保存装备数据
bool Profession::savePackageData(Cmd::ProfessionData& data)
{
  if(!m_pUser)
    return false;

  return m_pUser->getPackage().saveProfessionData(data);
}

// 加载Being数据
bool Profession::loadBeingData(const Cmd::BlobUserBeing& data)
{
  if(!m_pUser)
    return false;

  return m_pUser->getUserBeing().loadProfessionData(data);
}

// 保存Being数据
bool Profession::saveBeingData(Cmd::BlobUserBeing* pData)
{
  if(!m_pUser || !pData)
    return false;

  return m_pUser->getUserBeing().save(pData);
}

bool Profession::loadPartnerData(const Cmd::BlobPet& data)
{
  if(!m_pUser)
    return false;

  return m_pUser->getPet().load(data);
}

bool Profession::savePartnerData(Cmd::BlobPet* pData)
{
  if(!m_pUser || !pData)
    return false;

  return m_pUser->getPet().save(pData);
}

bool Profession::loadExchangeShopData(const Cmd::ProfessionData& data)
{
  return m_pUser->getExchangeShop().loadProfessionData(data);
}

bool Profession::saveExchangeShopData(Cmd::ProfessionData& data)
{
  return m_pUser->getExchangeShop().saveProfessionData(data);
}

void Profession::exchangeSkillcut(Cmd::ProfessionData& data)
{
  const SkillValidPosData& rPosData = data.skillpos();
  SkillValidPos* pData = data.mutable_shortcut();
  if (rPosData.pos_size() == 0 && rPosData.autopos_size() == 0 && rPosData.extendpos_size() == 0)
  {
    if (pData->shortcuts_size() > ESKILLSHORTCUT_MAX)
    {
      XLOG << "[多职业-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "快捷键数据" << pData->ByteSize() << "清空" << XEND;
      map<ESkillShortcut, TVecDWORD> mapSkillcut;
      for (int i = 0; i < pData->shortcuts_size(); ++i)
      {
        const SkillValidShortcut& rCut = pData->shortcuts(i);
        auto m = mapSkillcut.find(rCut.type());
        if (m != mapSkillcut.end())
          continue;
        TVecDWORD& setPos = mapSkillcut[rCut.type()];
        setPos.resize(rCut.pos_size());
        for (int j = 0; j < rCut.pos_size(); ++j)
          setPos.push_back(rCut.pos(j));
      }

      pData->clear_shortcuts();
      for (auto &m : mapSkillcut)
      {
        SkillValidShortcut* pCut = pData->add_shortcuts();
        pCut->set_type(m.first);
        for (auto &v : m.second)
          pCut->add_pos(v);
      }
    }
    return;
  }

  XLOG << "[多职业-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "快捷键数据" << pData->ByteSize() << "清空" << XEND;
  pData->Clear();

  if (rPosData.pos_size() != 0)
  {
    SkillValidShortcut* pCut = pData->add_shortcuts();
    pCut->set_type(ESKILLSHORTCUT_NORMAL);
    for (int i = 0; i < rPosData.pos_size(); ++i)
      pCut->add_pos(rPosData.pos(i));
  }
  if (rPosData.autopos_size() != 0)
  {
    SkillValidShortcut* pCut = pData->add_shortcuts();
    pCut->set_type(ESKILLSHORTCUT_AUTO);
    for (int i = 0; i < rPosData.autopos_size(); ++i)
      pCut->add_pos(rPosData.autopos(i));
  }
  if (rPosData.extendpos_size() != 0)
  {
    SkillValidShortcut* pCut = pData->add_shortcuts();
    pCut->set_type(ESKILLSHORTCUT_EXTEND);
    for (int i = 0; i < rPosData.extendpos_size(); ++i)
      pCut->add_pos(rPosData.extendpos(i));
  }
  data.clear_skillpos();
}

bool Profession::setExchangeTime(DWORD branch, DWORD evo, DWORD dwTime)
{
  const SBranchCFG* pCFG = RoleConfig::getMe().getBranchCFG(branch);
  if (pCFG == nullptr)
  {
    XERR << "[多职业-转职时间]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "设置 branch :" << branch << "evo :" << evo << "时间失败,branch未在 Table_Branch.txt 表中找到" << XEND;
    return false;
  }

  ProfessionSvrData& rData = m_mapSvrBranch[branch];
  rData.set_id(branch);

  if (evo == 1)
    rData.set_bepro_1_time(dwTime);
  else if (evo == 2)
    rData.set_bepro_2_time(dwTime);
  else if (evo == 3)
    rData.set_bepro_3_time(dwTime);
  else
  {
    XERR << "[多职业-转职时间]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "设置 branch :" << branch << "evo :" << evo << "时间失败,evo不合法" << XEND;
    return false;
  }

  XDBG << "[多职业-转职时间]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "设置 branch :" << branch << "evo :" << evo << "时间成功" << dwTime << XEND;
  return true;
}

bool Profession::getExchangeTime(DWORD branch, DWORD evo, DWORD& rTime)
{
  rTime = 0;
  auto m = m_mapSvrBranch.find(branch);
  if (m == m_mapSvrBranch.end())
    return false;

  if (evo == 1)
    rTime = m->second.bepro_1_time();
  else if (evo == 2)
    rTime = m->second.bepro_2_time();
  else if (evo == 3)
    rTime = m->second.bepro_3_time();
  else
  {
    XERR << "[多职业-转职时间]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "获取 branch :" << branch << "evo :" << evo << "时间失败,evo不合法" << XEND;
    return false;
  }

  XDBG << "[多职业-转职时间]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "获取 branch :" << branch << "evo :" << evo << "时间成功" << rTime << XEND;
  return true;
}

// 切换职业分支
bool Profession::changeBranch(DWORD dwBranchDest)
{
  if(!m_pUser)
    return false;

  DWORD dwBranchCur = m_pUser->getBranch();
  if(!hasBranch(dwBranchCur))
  {
    // 第一次切换分支
    ProfessionData& data_first = m_mapBranch[dwBranchCur];
    data_first.set_id(dwBranchCur);
    data_first.set_type(Cmd::ETypeBranch);
    XLOG << "[多职业-切换职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "first set current branch! branch:" << dwBranchCur << XEND;
  }

  // 同步同系分支joblv
  syncBranchJoblv(dwBranchCur, m_pUser->getJobLv());

  // 保存当前分支数据
  Cmd::ProfessionData data_cur;
  data_cur.CopyFrom(m_mapBranch[dwBranchCur]);
  if(!saveProfessionData(data_cur))
  {
    XERR << "[多职业-切换职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "save branch data failed, stop change branch! branch:" << dwBranchCur << XEND;
    return false;
  }

  data_cur.set_opertime(now());
  m_mapBranch[dwBranchCur] = data_cur;
  backup(data_cur);

  // 加载目标分支数据
  Cmd::ProfessionData& data_dest = m_mapBranch[dwBranchDest];
  if(!loadProfessionData(data_dest, data_dest.isfirst()))
  {
    XERR << "[多职业-切换职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "严重错误: 加载目标分支失败，回退分支数据，目标分支：" << dwBranchDest << XEND;
    if(!loadProfessionData(data_cur, false))
    {
      XERR << "[多职业-切换职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "致命错误：切换失败，回退当前分支失败! 当前分支:" << dwBranchCur << XEND;
      return false;
    }
    return false;
  }

  // 同步预览信息
  sendBranchData(dwBranchCur);

  const SBranchCFG* pCurCfg = RoleConfig::getMe().getBranchCFG(dwBranchCur);
  const SBranchCFG* pDestCfg = RoleConfig::getMe().getBranchCFG(dwBranchDest);
  if(!pCurCfg || !pDestCfg)
  {
    XERR << "[多职业-切换职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get branch table failed! curBranch:" << dwBranchCur << "destbranch:" << dwBranchDest << XEND;
    return false;
  }

  // 如果目标分支为二转以上职业 直接跳过
  // 如果转职任务已完成 则移除对应转职任务
  if(m_pUser->getEvo() == 1)
  {
    if(m_pUser->getQuest().isSubmit(pDestCfg->taskProfession))
    {
      // 删除目标分支任务
      for(auto& q : pDestCfg->vecDelTask)
        m_pUser->getQuest().abandonGroup(q, true);
    }
    XLOG << "[多职业-切换职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "delete task, branch:" << dwBranchDest << XEND;
  }

  // 巅峰等级任务
  if(m_pUser->getQuest().isSubmit(pCurCfg->taskPeak))
    m_pUser->getQuest().finishQuest(pDestCfg->taskPeak);

  m_dwLastLoadTime = now() + MiscConfig::getMe().getProfessionMiscCFG().dwLoadCD;
  m_pUser->setDataMark(EUSERDATATYPE_CUR_MAXJOB);
  m_pUser->refreshDataAtonce();

  XLOG << "[多职业-切换职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "change branch success! branch:" << dwBranchDest << XEND;
  return true;
}

void Profession::syncBranchJoblv(DWORD dwBranch, DWORD dwJoblv)
{
  if(!m_pUser)
    return;

  if(dwJoblv > ONE_GRADE_PRO_MAX_JOBLV)
    dwJoblv = ONE_GRADE_PRO_MAX_JOBLV;

  const SBranchCFG* pBranchCfg = RoleConfig::getMe().getBranchCFG(dwBranch);
  if(!pBranchCfg)
  {
    XERR << "[多职业-同步分支joblv]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get SBranchCFG failed! branch:" << dwBranch << XEND;
    return;
  }

  for(auto& v : pBranchCfg->vecBrotherId)
  {
    if(!hasBranch(v))
      continue;

    DWORD lv = getJobLv(v);
    if(ONE_GRADE_PRO_MAX_JOBLV <= lv)
      continue;

    if(lv >= dwJoblv)
      continue;

    setJobLv(v, dwJoblv);
  }
}

void Profession::onEquipExchange(const std::string& guidOld, const std::string& guidNew)
{
  if(!m_pUser)
    return;

  for(auto& m : m_mapBranch)
  {
    for(int i=0; i<m.second.pack_data_size(); ++i)
    {
      Cmd::EquipPackData* pDataPack = m.second.mutable_pack_data(i);
      if(!pDataPack)
      {
        XERR << "[多职业-置换装备]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get Cmd::EquipPackData failed! i:" << i << " old guid:" << guidOld << "new guid:" << guidNew << XEND;
        continue;
      }

      for(int j=0; j<pDataPack->datas_size(); ++j)
      {
        Cmd::EquipInfo* pDataEquip = pDataPack->mutable_datas(j);
        if(!pDataEquip)
        {
          XERR << "[多职业-置换装备]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get Cmd::EquipInfo failed! j:" << j << " old guid:" << guidOld << "new guid:" << guidNew << XEND;
          continue;
        }

        if(pDataEquip->guid() == guidOld)
        {
          pDataEquip->set_guid(guidNew);
          ItemBase* pItemBase = m_pUser->getPackage().getItem(guidNew);
          if(pItemBase)
            pDataEquip->set_type_id(pItemBase->getTypeID());
        }
      }
    }
  }
}

void Profession::getProfessions(TVecDWORD& vecProfessions)
{
  if(!m_pUser)
    return;

  for(auto& m : m_mapBranch)
  {
    if(m.first == m_pUser->getBranch())
      continue;

    const SBranchCFG* pBranchCfg = RoleConfig::getMe().getBranchCFG(m.first);
    if(!pBranchCfg)
    {
      XERR << "[多职业-获取已有职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get SBranchCFG failed! branch:" << m.first << XEND;
      continue;
    }

    for(auto& v : pBranchCfg->vecProfession)
    {
      if(m.second.profession() < v)
        continue;
      auto it = std::find(vecProfessions.begin(), vecProfessions.end(), v);
      if(vecProfessions.end() == it)
        vecProfessions.push_back(v);
    }
  }

  const SBranchCFG* pBranchCfg = RoleConfig::getMe().getBranchCFG(m_pUser->getBranch());
  if(!pBranchCfg)
  {
    XERR << "[多职业-获取已有职业]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get SBranchCFG failed! branch:" << m_pUser->getBranch() << XEND;
    return;
  }

  for(auto& v : pBranchCfg->vecProfession)
  {
    if(m_pUser->getProfession() < v)
      continue;
    auto it = std::find(vecProfessions.begin(), vecProfessions.end(), v);
    if(vecProfessions.end() == it)
      vecProfessions.push_back(v);
  }
}

//void Profession::collectAttr(TVecAttrSvrs& attrs)
void Profession::collectAttr()
{
  if(!m_pUser)
    return;

  Attribute* pAttr = m_pUser->getAttribute();
  if (pAttr == nullptr)
    return;

  // 获取已有职业列表
  TVecDWORD vecProfessions;
  getProfessions(vecProfessions);
  // 计算职业属性
  for(auto& v : vecProfessions)
  {
    const SRoleBaseCFG* pRoleCfg = RoleConfig::getMe().getRoleBase(static_cast<EProfession>(v));
    if(!pRoleCfg)
    {
      XERR << "[多职业-计算属性]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get SRoleBaseCFG failed! profession:" << v << XEND;
      continue;
    }
    
    if(!pRoleCfg->checkGender(m_pUser->getUserSceneData().getGender()))
      continue;

    for(auto& attr : pRoleCfg->vecUnlockAttr)
      pAttr->modifyCollect(ECOLLECTTYPE_PROFESSION, attr);
    //{
    //  float value = attrs[attr.type()].value() + attr.value();
    //  attrs[attr.type()].set_value(value);
    //}
  }
}

void Profession::backup(Cmd::ProfessionData& rData)
{
  if(!m_pUser)
    return;

  Cmd::ProfessionSaveRecordCmd cmd;
  cmd.set_charid(m_pUser->id);
  cmd.set_branch(rData.id());

  string data;
  if(!rData.SerializeToString(&data))
  {
    XERR << "[多职业-备份分支]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "serialize failed! branch:" << rData.id() << XEND;
    return;
  }
  if(!compress(data, data))
  {
    XERR << "[多职业-备份分支]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "compress failed! branch:" << rData.id() << XEND;
    return;
  }
  cmd.set_data(data.c_str(), data.size());

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToData(send, len);
}

void Profession::restore()
{
  if(!m_pUser || !thisServer)
    return;

  Cmd::ProfessionQueryRecordCmd cmd;
  cmd.set_charid(m_pUser->id);
  cmd.set_scenename(thisServer->getServerName());

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToData(send, len);
}

void Profession::fixBranch(Cmd::ProfessionQueryRecordCmd& cmd)
{
  for(int i=0; i<cmd.datas_size(); ++i)
  {
    string data;
    if(!uncompress(cmd.datas(i), data))
      continue;

    Cmd::ProfessionData oData;
    if(!oData.ParseFromString(data))
      continue;

    m_mapBranch[oData.id()] = oData;
    XLOG << "[多职业-读取备份分支] success! branch:" << oData.id() << XEND;
  }
}

bool Profession::remove(DWORD branch)
{
  if(!m_pUser)
    return false;

  if(m_pUser->getBranch() == branch)
    return false;

  m_mapBranch.erase(branch);
  return true;
}


