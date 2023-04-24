#include "SceneFighter.h"
#include "SceneUser.h"
#include "MiscConfig.h"
#include "LuaManager.h"
#include "SceneServer.h"
#include "MailManager.h"
#include "PlatLogManager.h"
#include "StatisticsDefine.h"
#include "Menu.h"

SceneFighter::SceneFighter(SceneUser* pUser, const SRoleBaseCFG* pCFG) : m_pUser(pUser), m_pCFG(pCFG), m_oSkill(this)
{
  m_bBuy = false;
}

SceneFighter::~SceneFighter()
{

}

void SceneFighter::toData(FighterInfo* pInfo)
{
  if (pInfo == nullptr)
    return;

  UserData* pData = pInfo->add_datas();
  if (pData != nullptr)
  {
    pData->set_type(EUSERDATATYPE_PROFESSION);
    pData->set_value(getProfession());
  }
  pData = pInfo->add_datas();
  if (pData != nullptr)
  {
    pData->set_type(EUSERDATATYPE_JOBLEVEL);
    pData->set_value(getJobLv());
  }
  pData = pInfo->add_datas();
  if (pData != nullptr)
  {
    pData->set_type(EUSERDATATYPE_JOBEXP);
    pData->set_value(getJobExp());
  }
}

void SceneFighter::setTotalPoint(DWORD point)
{
  XLOG << "[玩家角色]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "设置素质点 原=" << m_dwTotalPoint << "新=" << point << XEND;
  m_dwTotalPoint = point;
  m_pUser->setDataMark(EUSERDATATYPE_TOTALPOINT);
  m_pUser->getEvent().onRolePointChange();
}

void SceneFighter::setHp(DWORD hp)
{
  DWORD maxhp = m_pUser->getAttr(EATTRTYPE_MAXHP);
  hp = hp > maxhp ? maxhp : hp;

  if (m_dwHp == hp)
    return;

  m_pUser->setAttr(EATTRTYPE_HP, hp);
  m_dwHp = hp;

  m_pUser->refreshDataAtonce();
}

void SceneFighter::setSp(DWORD sp)
{
  DWORD maxsp = m_pUser->getAttr(EATTRTYPE_MAXSP);
  sp = sp > maxsp ? maxsp : sp;

  if (m_dwSp == sp)
    return;

  m_pUser->setAttr(EATTRTYPE_SP, sp);
  m_dwSp = sp;

  m_pUser->refreshDataAtonce();
}

void SceneFighter::setProfession(EProfession eProfession)
{
  m_eProfession = eProfession;
}

void SceneFighter::setJobExp(QWORD exp)
{
  if (m_qwJobExp == exp)
    return;
  m_qwJobExp = exp;
  m_pUser->setDataMark(EUSERDATATYPE_JOBEXP);
}

void SceneFighter::setJobLv(DWORD lv)
{
  m_dwJobLv = lv;
  m_pUser->setDataMark(EUSERDATATYPE_JOBLEVEL);
}

DWORD SceneFighter::getMaxJobLv() const
{
  if(!m_pCFG || !m_pUser)
    return 0;

  if(4 == m_pUser->getEvo())
    return m_pCFG->maxJobLv + m_pUser->getUserSceneData().getDeadLv();

  if(m_pCFG->maxJobLv >= m_dwMaxJobLv)
    return m_pCFG->maxJobLv;

  return m_dwMaxJobLv;
}

// 同步最大等级（多职业切换用）
void SceneFighter::syncMaxJobLv(DWORD dwMaxJobLv)
{
  if(!m_pCFG || !m_pCFG->dwPeakJobLv)
    return;

  if(dwMaxJobLv >= m_pCFG->dwPeakJobLv)
    m_dwMaxJobLv = m_pCFG->dwPeakJobLv;
  else if(getMaxJobLv() < dwMaxJobLv)
    m_dwMaxJobLv = dwMaxJobLv;
}

void SceneFighter::setMaxJobLv(DWORD lv)
{
  if(m_pCFG->dwPeakJobLv == 0)
    return;

  if(m_pUser->getMenu().isOpen(EMENUID_PEAK_LEVEL) == false || MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_PEAK))
    return;

  DWORD dwMax = (m_pCFG->dwPeakJobLv > m_pCFG->maxJobLv) ? m_pCFG->dwPeakJobLv : m_pCFG->maxJobLv;
  if(getMaxJobLv() >= dwMax)
  {
    m_dwMaxJobLv = dwMax;
    return;
  }

  if(getMaxJobLv() + lv > dwMax)
    m_dwMaxJobLv = dwMax;
  else
    m_dwMaxJobLv = getMaxJobLv() + lv;

  m_pUser->setMaxJobLv(m_dwMaxJobLv);
  m_pUser->setDataMark(EUSERDATATYPE_CUR_MAXJOB);
  m_pUser->refreshDataAtonce();
  m_pUser->getServant().onFinishEvent(ETRIGGER_MAXJOBLEVEL);
  MsgManager::sendMsg(m_pUser->id, 3435, MsgParams(m_dwMaxJobLv - 90));
  XLOG << "[玩家角色-最大Job值] 提升" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "maxJobLv: " <<m_dwMaxJobLv << XEND;
}

void SceneFighter::reqAddMaxJobCmd(const AddJobLevelItemCmd& cmd)
{
  if(m_pUser->getMenu().isOpen(EMENUID_PEAK_LEVEL) == false || MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_PEAK))
    return;

  if(3 > m_pUser->getEvo())
    return;

  DWORD dwMax = (m_pCFG->dwPeakJobLv > m_pCFG->maxJobLv) ? m_pCFG->dwPeakJobLv : m_pCFG->maxJobLv;
  if(getMaxJobLv() >= dwMax)
    return;

  DWORD addlv = MiscConfig::getMe().getAddLvByItem(cmd.item(), cmd.num());
  if(addlv != 0)
  {
    MainPackage* pPackage = dynamic_cast<MainPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_MAIN));
    if (pPackage == nullptr)
      return ;
    if(pPackage->checkItemCount(cmd.item(), cmd.num()) == false)
      return ;
    pPackage->reduceItem(cmd.item(), ESOURCE_PACKAGE, cmd.num());

    setMaxJobLv(addlv);
  }
}

DWORD SceneFighter::getBattlePoint()
{
  /*xSceneEntryDynamic* pEntry = m_pUser;
  DWORD dwBattlePoint = LuaManager::getMe().call<float>("calcBattlePoint", pEntry, m_pUser->getLevel(), m_pUser->getProfession(), m_oSkill.getSkillBTPoint());
  if (m_dwBattlePoint == dwBattlePoint)
    return m_dwBattlePoint;

  m_dwBattlePoint = dwBattlePoint;
  m_pUser->setDataMark(EUSERDATATYPE_BATTLEPOINT);

  XLOG << "[玩家角色-战斗力同步]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "同步战斗力=" << m_dwBattlePoint << XEND;*/
  return m_dwBattlePoint;
}

bool SceneFighter::fromRoleData(const UserRoleData& data)
{
  // base data
  m_qwJobExp = data.jobexp();
  m_dwJobLv = data.joblv();
  m_dwMaxJobLv = data.maxjoblv();

  m_eProfession = data.profession();
  m_dwBranch = data.branch();
  m_bBuy = data.isbuy();

  // attr point
  m_dwStrPoint = data.strpoint();
  m_dwIntPoint = data.intpoint();
  m_dwAgiPoint = data.agipoint();
  m_dwDexPoint = data.dexpoint();
  m_dwVitPoint = data.vitpoint();
  m_dwLukPoint = data.lukpoint();
  m_dwTotalPoint = data.totalpoint();
  m_dwUsedPoint = data.usedpoint();
  m_dwHp = data.hp();
  m_dwSp = data.sp();

  // skill
  if (m_oSkill.load(data.skill()) == false)
    return false;

  // unlocklv
  m_vecUnlockLv.clear();
  for (int i = 0; i < data.unlocklv_size(); ++i)
    m_vecUnlockLv.push_back(data.unlocklv(i));

  return true;
}

bool SceneFighter::toRoleData(UserRoleData& data)
{
  // base data
  data.set_jobexp(m_qwJobExp);
  data.set_joblv(m_dwJobLv);
  data.set_maxjoblv(m_dwMaxJobLv);

  data.set_profession(m_eProfession);
  data.set_branch(m_dwBranch);
  data.set_isbuy(m_bBuy);

  // attr point
  data.set_strpoint(m_dwStrPoint);
  data.set_intpoint(m_dwIntPoint);
  data.set_agipoint(m_dwAgiPoint);
  data.set_dexpoint(m_dwDexPoint);
  data.set_vitpoint(m_dwVitPoint);
  data.set_lukpoint(m_dwLukPoint);
  data.set_totalpoint(m_dwTotalPoint);
  data.set_usedpoint(m_dwUsedPoint);
  data.set_hp(m_dwHp);
  data.set_sp(m_dwSp);

  // skill
  if (m_oSkill.save(data.mutable_skill()) == false)
    return false;

  // unlocklv
  data.clear_unlocklv();
  for (auto v = m_vecUnlockLv.begin(); v != m_vecUnlockLv.end(); ++v)
    data.add_unlocklv(*v);

  XDBG << "[玩家角色-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小:" << data.ByteSize() << XEND;
  return true;
}

// 加载多职业数据
bool SceneFighter::loadProfessionData(const UserRoleData& rData, bool isFirst)
{
  if(!m_pUser)
    return false;

  // 先重置
  resetAttrPoint();
  m_oSkill.resetSkill();
  const SBranchCFG* pCfgOld = RoleConfig::getMe().getBranchCFG(m_dwBranch);
  if(!pCfgOld)
  {
    XERR << "[玩家角色-多职业加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get SBranchCFG failed! branch:" << m_dwBranch << XEND;
    return false;
  }

  m_pUser->m_oBuff.setClearState();
  m_pUser->m_oBuff.refreshBuffAtonce();
  //m_pUser->m_oBuff.clear(); // 删除buff

  for(auto& v : pCfgOld->vecJobSkill)
    m_oSkill.removeSkill(v, 0, ESOURCE_PROFESSION_CHANGE, true); // 职业技能


  // 后加载
  const SRoleBaseCFG* pOldCFG = getRoleCFG();
  const SRoleBaseCFG* pDestCFG = RoleConfig::getMe().getRoleBase(rData.profession());

  setRoleCFG(pDestCFG);
  fromRoleData(rData);
  m_pUser->getUserSceneData().setProfession(m_eProfession);

  if(isFirst)
  {
    //m_oSkill.equipSkill(11001, 0);
    m_oSkill.equipSkill(ESKILLSHORTCUT_NORMAL, 11001, 0, 0);
  }

  // 无奈之举
  const SBranchCFG* pCfgNew = RoleConfig::getMe().getBranchCFG(m_dwBranch);
  if(!pCfgNew)
  {
    XERR << "[玩家角色-多职业加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "get new SBranchCFG failed! branch:" << m_dwBranch << XEND;
    return false;
  }

  for(auto& v : pCfgNew->vecJobSkill)
  {
    if(m_pUser->getEvo() >= 2)
      m_oSkill.addSkill(v, SKILL_SOURCEID, ESOURCE_NORMAL, false, false); // 职业技能
  }

  if(m_oSkill.getReplaceNormalSkill())
    m_oSkill.restoreNormalSkill();

  if(pOldCFG->normalSkill != pDestCFG->normalSkill)
  {
    m_oSkill.removeSkill(pOldCFG->normalSkill, 0, ESOURCE_MIN);
    m_oSkill.addSkill(pDestCFG->normalSkill, 0, ESOURCE_MIN, false);
  }

  if(pOldCFG->strengthSkill != pDestCFG->strengthSkill)
  {
    m_oSkill.removeSkill(pOldCFG->strengthSkill, 0, ESOURCE_MIN);
    m_oSkill.addSkill(pDestCFG->strengthSkill, 0, ESOURCE_MIN, false);
  }

  const SNewRoleCFG& rCFG = MiscConfig::getMe().getNewRoleCFG();
  const TSetDWORD& oldbuf = rCFG.getClassInitBuff(pOldCFG->profession);
  const TSetDWORD& newbuf = rCFG.getClassInitBuff(pDestCFG->profession);
  for(auto& s : oldbuf)
    m_pUser->m_oBuff.del(s);
  for(auto& s : newbuf)
    m_pUser->m_oBuff.add(s);

  m_oSkill.refreshEnableSkill();
  m_oSkill.clearUpdate();

  /*ReqSkillData cmd;
  m_oSkill.toClient(cmd);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);*/
  m_oSkill.sendSkillData();

  // 设置属性点
  setTotalPoint(m_pUser->getTotalPoint() - m_dwUsedPoint);

  // 设置技能栏
  m_oSkill.setMaxPos(m_pUser->getMaxSkillPos());
  m_oSkill.setAutoMaxPos(m_pUser->getAutoSkillPos());
  m_oSkill.setMaxExtendPos(m_pUser->getExtendSkillPos());

  // 同步最大joblv
  syncMaxJobLv(m_pUser->getMaxJobLv());

  // 加载技能buff
  m_pUser->m_oBuff.loadSkillBuff();

  /*SkillValidPos cmd1;
  m_oSkill.toClient(cmd1);
  PROTOBUF(cmd1, send1, len1);
  m_pUser->sendCmdToMe(send1, len1);*/
  m_oSkill.sendValidPos();
  m_pUser->getMenu().refreshNewMenu(EMENUCOND_SKILL);

#ifdef _DEBUG
  XLOG << "[玩家角色-多职业加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "load fighter data success!" << XEND;
#endif
  return true;
}

bool SceneFighter::saveProfessionData(UserRoleData* pData)
{
  if(!m_pUser || !pData)
    return false;

  pData->Clear();
  return toRoleData(*pData);
}

bool SceneFighter::addAttrPoint(EUserDataType eType, DWORD point)
{
  // check total point
  if (m_dwTotalPoint <= 0)
    return false;

  // get point value
  DWORD* p = nullptr;
  if (eType == EUSERDATATYPE_STRPOINT)
    p = &m_dwStrPoint;
  else if (eType == EUSERDATATYPE_INTPOINT)
    p = &m_dwIntPoint;
  else if (eType == EUSERDATATYPE_AGIPOINT)
    p = &m_dwAgiPoint;
  else if (eType == EUSERDATATYPE_DEXPOINT)
    p = &m_dwDexPoint;
  else if (eType == EUSERDATATYPE_VITPOINT)
    p = &m_dwVitPoint;
  else if (eType == EUSERDATATYPE_LUKPOINT)
    p = &m_dwLukPoint;
  if (p == nullptr)
    return false;

  // check max point
  /*const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();
  if (*p >= rCFG.dwMaxAttrPoint)
  {
    XERR("[玩家-增加素质点] %llu, %llu, %u, %s, type : %u 超级最大素质点%u", m_pUser->accid, m_pUser->id, m_pUser->getProfession(), m_pUser->name, eType, rCFG.dwMaxAttrPoint);
    return false;
  }*/
  
  DWORD dwOld = *p;
  // get config
  DWORD dwAddPoint = point;
  DWORD dwRealPoint = 0;
  while (point > 0)
  {
    const PAttr2Point* pCFG = AttributePointConfig::getMe().getAttrPointCFG(*p + 1);
    if (pCFG == nullptr)
      break;

    if (m_dwTotalPoint < pCFG->second)
      break;

    m_dwTotalPoint -= pCFG->second;
    m_dwUsedPoint += pCFG->second;
    *p += 1;
    dwRealPoint += 1;
    if (--point <= 0)
      break;

    /*if (*p >= rCFG.dwMaxAttrPoint)
    {
      XERR("[玩家-增加素质点] %llu, %llu, %u, %s, type = : %u 超过最大素质点%u", m_pUser->accid, m_pUser->id, m_pUser->getProfession(), m_pUser->name, eType, rCFG.dwMaxAttrPoint);
      break;
    }*/
  }

  // update attribute
  m_pUser->setCollectMark(ECOLLECTTYPE_BASE);

  // save data
  m_pUser->setDataMark(EUSERDATATYPE_TOTALPOINT);
  m_pUser->setDataMark(EUSERDATATYPE_USEDPOINT);
  m_pUser->setDataMark(eType);

  // update data
  m_pUser->refreshDataAtonce();

  XLOG << "[玩家角色-增加素质点]" << m_pUser->accid << m_pUser->id << m_eProfession << m_pUser->name
    << "增加 type:" << eType << "point:" << dwAddPoint << "realpoint:" << dwRealPoint << "totalpoint:" << *p << XEND;

  //plat log
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eventType = EventType_Change_Attr;
  PlatLogManager::getMe().eventLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eid,
    m_pUser->getUserSceneData().getCharge(), eventType, 0, 1);

  PlatLogManager::getMe().changeLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eventType,
    eid,/*eid*/
    EChange_Attr, dwOld, *p, eType);

  return true;
}

bool SceneFighter::resetAttrPoint()
{
  // collect all point
  if (m_dwUsedPoint == 0)
    return false;

  // recover point to total point
  m_dwTotalPoint += m_dwUsedPoint;

  // reset point
  m_dwStrPoint = m_dwIntPoint = m_dwAgiPoint = m_dwDexPoint = m_dwVitPoint = m_dwLukPoint = m_dwUsedPoint = 0;

  // update attribute
  m_pUser->setCollectMark(ECOLLECTTYPE_BASE);
  m_pUser->setDataMark(EUSERDATATYPE_TOTALPOINT);
  m_pUser->setDataMark(EUSERDATATYPE_USEDPOINT);
  m_pUser->setDataMark(EUSERDATATYPE_STRPOINT);
  m_pUser->setDataMark(EUSERDATATYPE_INTPOINT);
  m_pUser->setDataMark(EUSERDATATYPE_AGIPOINT);
  m_pUser->setDataMark(EUSERDATATYPE_DEXPOINT);
  m_pUser->setDataMark(EUSERDATATYPE_VITPOINT);
  m_pUser->setDataMark(EUSERDATATYPE_LUKPOINT);

  // update data
  m_pUser->refreshDataAtonce();

  m_pUser->getTip().onRolePoint();

  XLOG << "[玩家角色-重置素质点]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "总素质点=" << m_dwTotalPoint << XEND;

  //plat log
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eventType = EventType_Change_ResetAttrPoint;
  PlatLogManager::getMe().eventLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eid,
    m_pUser->getUserSceneData().getCharge(), eventType, 0, 1);

  PlatLogManager::getMe().changeLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eventType,
    eid,/*eid*/
    EChange_ResetAttrPoint, 0, 0, 0);

  StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_RESET_POINT_SUM, m_pUser->getProfession(),ATTR_POINT_REST, 0, (DWORD)1);
  if (m_pUser->m_oUserStat.checkAndSet(ESTATTYPE_RESET_POINT, ATTR_POINT_REST))
    m_pUser->m_oUserStat.sendStatLog(ESTATTYPE_RESET_POINT, m_pUser->getProfession(), ATTR_POINT_REST, 0, (DWORD)1);

  return true;
}

DWORD SceneFighter::getAttrPoint(EAttrType eType)
{
  if (eType == EATTRTYPE_STR)
    return m_dwStrPoint;
  if (eType == EATTRTYPE_INT)
    return m_dwIntPoint;
  if (eType == EATTRTYPE_AGI)
    return m_dwAgiPoint;
  if (eType == EATTRTYPE_DEX)
    return m_dwDexPoint;
  if (eType == EATTRTYPE_VIT)
    return m_dwVitPoint;
  if (eType == EATTRTYPE_LUK)
    return m_dwLukPoint;

  return 0;
}

void SceneFighter::checkUnlock()
{
  if (m_pUser == nullptr || m_pUser->getRoleBaseCFG() == nullptr)
    return;

  const SRoleBaseCFG* pCFG = m_pUser->getRoleBaseCFG();
  for (auto v = pCFG->vecUnlock.begin(); v != pCFG->vecUnlock.end(); ++v)
  {
    if (m_dwJobLv < v->dwJobLv)
      continue;

    auto o = find(m_vecUnlockLv.begin(), m_vecUnlockLv.end(), v->dwJobLv);
    if (o != m_vecUnlockLv.end())
      continue;

    auto sendreward = [&](DWORD dwMailID) -> bool
    {
      if (m_pUser == nullptr)
        return false;

      return MailManager::getMe().sendMail(m_pUser->id, dwMailID);
    };

    bool bSuccess = false;
    DWORD dwMailID = 0;
    if (v->dwMaleMailID != 0 && m_pUser->getUserSceneData().getGender() == EGENDER_MALE)
    {
      bSuccess = sendreward(v->dwMaleMailID);
      dwMailID = v->dwMaleMailID;
    }
    else if (v->dwFemaleMailID != 0 && m_pUser->getUserSceneData().getGender() == EGENDER_FEMALE)
    {
      bSuccess = sendreward(v->dwFemaleMailID);
      dwMailID = v->dwFemaleMailID;
    }

    if (!bSuccess)
    {
      XERR << "[玩家角色-解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "mailid=" << dwMailID << "未在Table_Mail.txt表中找到 job=" << m_pUser->getJobLv() << "no unlock" << XEND;
      continue;
    }

    m_vecUnlockLv.push_back(v->dwJobLv);
    XLOG << "[玩家角色-解锁]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "job=" << m_pUser->getJobLv() << "解锁" << XEND;
  }
}

void SceneFighter::timer(DWORD curTime)
{
  m_oSkill.update(curTime);
}

