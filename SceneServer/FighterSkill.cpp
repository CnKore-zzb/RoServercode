#include "FighterSkill.h"
#include "SceneFighter.h"
#include "SceneUser.h"
#include "UserConfig.h"
#include "SkillManager.h"
#include "MiscConfig.h"
#include "MsgManager.h"
#include "PlatLogManager.h"
#include "SceneServer.h"
#include "StatisticsDefine.h"
#include "SceneSkill.pb.h"
#include "AstrolabeConfig.h"
#include "Menu.h"

// skill consume
void SSkillConsume::toData(SkillConsume* pConsume, SceneFighter* pSF)
{
  if (pConsume == nullptr || pSF == nullptr)
    return;
  //const SPurifyCFG& purifyCFG = MiscConfig::getMe().getSPurifyCFG();
  DWORD maxvalue = MiscConfig::getMe().getSPurifyCFG().dwMaxPurify;
  pConsume->set_curvalue(pSF->getUser()->getPurify());
  pConsume->set_maxvalue(maxvalue);

  pConsume->set_nexttime(nexttime);
}

void SSkillConsume::fromData(const SkillConsume& rConsume, SceneFighter* pSF)
{
  if (!pSF)
    return;
  nexttime = rConsume.nexttime();
}

void SSkillConsume::reset()
{
  DWORD interval = MiscConfig::getMe().getSPurifyCFG().dwGainInterval;
  if (nexttime == 0)
    nexttime = now() +interval;
}

bool SSkillItem::isRealget() const
{
  return eSource == ESOURCE_MIN || eSource == ESOURCE_SHOP || eSource == ESOURCE_NORMAL;
}

// skill item
bool SSkillItem::toData(SkillItem* pItem, SceneFighter* pSF)
{
  if (pItem == nullptr || pSF == nullptr)
    return false;

  pItem->set_id(dwID);
  //pItem->set_pos(pSF->getSkill().getPos(dwID, dwSourceid));
  pItem->set_cd(dwCD);
  pItem->set_active(bActive);
  pItem->set_learn(bLearn);
  pItem->set_profession(eProfession);
  pItem->set_source(eSource);
  pItem->set_sourceid(dwSourceid);
  //pItem->set_autopos(pSF->getSkill().getAutoPos(dwID, dwSourceid));
  pItem->set_shadow(bShadow);
  //pItem->set_extendpos(pSF->getSkill().getExtendPos(dwID, dwSourceid));
  if (dwRuneSpecID)
    pItem->set_runespecid(dwRuneSpecID);
  if (dwReplaceID)
    pItem->set_replaceid(dwReplaceID);
  if (dwExtraLv)
    pItem->set_extralv(dwExtraLv);
  pItem->set_selectswitch(bRuneOpened);

  pItem->set_ownerid(qwOwnerID);

  pItem->clear_shortcuts();
  for (DWORD d = ESKILLSHORTCUT_MIN + 1; d < ESKILLSHORTCUT_MAX; ++d)
  {
    if (d == ESKILLSHORTCUT_BEINGAUTO || ESkillShortcut_IsValid(d) == false)
      continue;
    ESkillShortcut eType = static_cast<ESkillShortcut>(d);
    SPosData* pData = pSF->getSkill().getShortcut(eType, dwID, dwSourceid);
    if (pData == nullptr || pData->dwPos == 0)
      continue;
    SkillShortcut* pCut = pItem->add_shortcuts();
    pCut->set_type(eType);
    pCut->set_pos(pData->dwPos);
  }

  if (pSkillCFG == nullptr || !pSkillCFG->isConsumeSkill())
    return false;
  SkillConsume* pConsume = pItem->mutable_consume();
  if (!pConsume)
    return false;
  consume.toData(pConsume, pSF);

  return true;
}

bool SSkillItem::fromData(const SkillItem& rItem, SceneFighter* pSF)
{
  if (pSF == nullptr)
    return false;

  dwID = rItem.id();
  //dwPos = rItem.pos();
  dwCD = rItem.cd();
  //dwAutoPos = rItem.autopos();

  bActive = rItem.active();
  bLearn = rItem.learn();
  bShadow = rItem.shadow();

  eProfession = rItem.profession();
  eSource = rItem.source();

  dwSourceid = rItem.sourceid();

  dwRuneSpecID = rItem.runespecid();
  //dwReplaceID = rItem.replaceid(); 不保存
  //dwExtraLv = rItem.extralv(); 不保存
  bRuneOpened = rItem.selectswitch();

  qwOwnerID = rItem.ownerid();

  pSkillCFG = SkillManager::getMe().getSkillCFG(dwID);
  if (pSkillCFG == nullptr)
    return false;

  const SkillConsume& rConsume = rItem.consume();
  consume.fromData(rConsume, pSF);

  mapShortcut.clear();
  for (int i = 0; i < rItem.shortcuts_size(); ++i)
  {
    const SkillShortcut& rCut = rItem.shortcuts(i);
    mapShortcut[rCut.type()] = rCut.pos();
  }

  if (rItem.pos() != 0)
    mapShortcut[ESKILLSHORTCUT_NORMAL] = rItem.pos();
  if (rItem.autopos() != 0)
    mapShortcut[ESKILLSHORTCUT_AUTO] = rItem.autopos();
  if (rItem.extendpos() != 0)
    mapShortcut[ESKILLSHORTCUT_EXTEND] = rItem.extendpos();

  return true;
}

// skill data
void SSkillData::toData(SkillData* pData, SceneFighter* pSF)
{
  if (pData == nullptr || pSF == nullptr)
    return;

  pData->set_usedpoint(dwUsedPoint);
  pData->set_profession(eProfession);
  pData->set_primarypoint(dwPrimaryPoint);

  pData->clear_items();
  DWORD replaceNormalSkill = pSF->getSkill().getReplaceNormalSkill();
  for (auto v = vecSkillItem.begin(); v != vecSkillItem.end(); ++v)
  {
    if (pSF->getProfession() == EPROFESSION_NOVICE && v->eSource == ESOURCE_MIN)
    {
      const SRoleBaseCFG* pCFG = pSF->getRoleCFG();
      if (pCFG != nullptr)
      {
        // 普攻, 重击不存数据库
        if (RoleConfig::getMe().isNormalSkill(v->dwID) || RoleConfig::getMe().isStrengthSkill(v->dwID))
          continue;

        // 无效技能清除
        if (!pCFG->haveSkill(v->dwID))
          continue;
      }
    }
    // 临时添加的技能不存储
    if (!v->isRealget())
    {
      if (!pSF->getUser())
        continue;
      SceneFighter* pFighter = pSF->getUser()->getCurFighter();
      if (pFighter == nullptr)
        continue;
      if (pFighter->getSkill().isInSlot(v->dwID, v->dwSourceid) == false)
        continue;
    }
    if (v->dwID == replaceNormalSkill)
      continue;

    SkillItem* pItem = pData->add_items();
    if (pItem == nullptr)
    {
      XERR << "[SSkillData::toData] add_items error!" << XEND;
      continue;
    }

    // 共享技能存在acc中
    const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(v->dwID);
    if (pSkillCFG && pSkillCFG->isShareSkill())
      continue;

    v->toData(pItem, pSF);
    // 来自装备的技能, 均按残影(无效技能)保存, 上线时, 重新addSkill
    if (!v->isRealget() && v->bShadow == false)
    {
      pItem->set_shadow(true);
    }
  }
}

void SSkillData::toClientData(SkillData* pData, SceneFighter* pSF)
{
  if (pData == nullptr || pSF == nullptr)
    return;

  pData->set_usedpoint(dwUsedPoint);
  pData->set_profession(eProfession);
  pData->set_primarypoint(dwPrimaryPoint);

  pData->clear_items();
  for (auto v = vecSkillItem.begin(); v != vecSkillItem.end(); ++v)
  {
    SkillItem* pItem = pData->add_items();
    if (pItem == nullptr)
      continue;

    v->toData(pItem, pSF);
  }
}

void SSkillData::fromData(const SkillData& rData, SceneFighter* pSF)
{
  if (pSF == nullptr || pSF->getUser() == nullptr)
    return;

  dwUsedPoint = rData.usedpoint();
  eProfession = rData.profession();
  dwPrimaryPoint = rData.primarypoint();
  if(dwUsedPoint <= MiscConfig::getMe().getSkillBreakPoint() && dwUsedPoint != 0 && dwPrimaryPoint == 0)
    dwPrimaryPoint = dwUsedPoint;   //兼容老账号

  SceneUser* pUser = pSF->getUser();
  vecSkillItem.clear();
  for (int i = 0; i < rData.items_size(); ++i)
  {
    if (pSF->getProfession() == EPROFESSION_NOVICE && rData.items(i).source() == ESOURCE_MIN)
    {
      const SRoleBaseCFG* pCFG = pSF->getRoleCFG();
      if (pCFG != nullptr)
      {
        // 普攻, 重击不存数据库
        if (RoleConfig::getMe().isNormalSkill(rData.items(i).id()) || RoleConfig::getMe().isStrengthSkill(rData.items(i).id()))
          continue;
        // 无效技能清除
        if (!pCFG->haveSkill(rData.items(i).id()))
          continue;
      }
    }

    // 共享技能存在acc中
    const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(rData.items(i).id());
    if (pSkillCFG && pSkillCFG->isShareSkill())
      continue;

    SSkillItem stItem;
    if (stItem.fromData(rData.items(i), pSF) == false)
    {
      XERR << "[技能数据-加载]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "加载 skillid:" << rData.items(i).id() << "失败" << XEND;
      continue;
    }
    vecSkillItem.push_back(stItem);
  }
}

bool SSkillData::isSkillEnable(DWORD id)
{
  auto v = find_if(vecSkillItem.begin(), vecSkillItem.end(), [id](const SSkillItem& r) -> bool{
    if (r.dwReplaceID)
      return r.dwReplaceID == id && r.bLearn == true && !r.bShadow;
    if (r.dwExtraLv)
      return r.dwID + r.dwExtraLv == id && r.bLearn && !r.bShadow;
    return r.dwID == id && r.bLearn == true && !r.bShadow;
  });
  if (v != vecSkillItem.end())
    return true;
  return false;
}

bool SSkillData::isSkillFamilyEnable(DWORD familyid)
{
  auto v = find_if(vecSkillItem.begin(), vecSkillItem.end(), [familyid](const SSkillItem& r) -> bool{
    if (r.dwReplaceID)
      return r.dwReplaceID / ONE_THOUSAND == familyid && r.bLearn == true && !r.bShadow;
    return r.dwID / ONE_THOUSAND == familyid && r.bLearn == true && !r.bShadow;
  });
  if (v != vecSkillItem.end())
    return true;
  return false;
}

SSkillItem* SSkillData::getSkillItem(DWORD id, DWORD sourceid /*= 0*/, ESource eSource /*= ESOURCE_MIN*/)
{
  auto v = find_if(vecSkillItem.begin(), vecSkillItem.end(), [id, sourceid](const SSkillItem& r) -> bool{
    return r.dwID == id && r.dwSourceid == sourceid;
  });
  if (v != vecSkillItem.end())
    return &(*v);

  return nullptr;
}

/*
SSkillItem* SSkillData::getSkillItemByPos(DWORD pos, bool autoOrNormal)
{
  auto v = find_if(vecSkillItem.begin(), vecSkillItem.end(), [pos, autoOrNormal](const SSkillItem& r) -> bool{
    return autoOrNormal ? r.dwAutoPos == pos : r.dwPos == pos;
  });
  if (v != vecSkillItem.end())
    return &(*v);

  return nullptr;
}
*/

SSkillItem* SSkillData::getSkillByType(DWORD id)
{
  auto v = find_if(vecSkillItem.begin(), vecSkillItem.end(), [id](const SSkillItem& r) -> bool{
    return id / 100 == r.dwID / 100 && r.isRealget();
  });
  if (v != vecSkillItem.end())
    return &(*v);

  return nullptr;
}

bool SSkillData::removeSkill(DWORD id)
{
  auto v = find_if(vecSkillItem.begin(), vecSkillItem.end(), [id](const SSkillItem& r) -> bool{
   return r.dwID == id;
  });
  if (v == vecSkillItem.end())
    return false;

  vecSkillItem.erase(v);
  return true;
}

// fighter skill
FighterSkill::FighterSkill(SceneFighter* pFighter) : m_pFighter(pFighter)
{
  initShortcut();
}

FighterSkill::~FighterSkill()
{

}

bool FighterSkill::load(const UserSkillData& data)
{
  m_dwTotalPoint = data.totalpoint();
  m_dwMaxPos = data.maxpos();
  m_dwAutoMaxPos = data.automaxpos();
  m_dwMaxExtendPos = data.maxextendpos();

  m_mapShortcut.clear();
  initShortcut();

  m_vecSkillData.clear();
  for (int i = 0; i < data.datas_size(); ++i)
  {
    SSkillData stData;
    stData.fromData(data.datas(i), m_pFighter);
    m_vecSkillData.push_back(stData);
  }

  SceneUser* pUser = m_pFighter->getUser();
  for (int i = 0; i < data.shortcuts_size(); ++i)
  {
    const SkillShortcutDB& rCut = data.shortcuts(i);
    auto m = m_mapShortcut.find(rCut.type());
    if (m == m_mapShortcut.end())
    {
      XERR << "[角色技能-加载]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "快捷栏" << rCut.ShortDebugString() << "加载失败,未找到该类型快捷栏" << XEND;
      continue;
    }
    for (int j = 0; j < rCut.cuts_size(); ++j)
    {
      const SkillPos& rPos = rCut.cuts(j);
      if (rPos.pos() == 0)
      {
        XERR << "[角色技能-加载]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "快捷栏" << rCut.ShortDebugString() << "加载失败,栏位为0" << XEND;
        continue;
      }
      if (rPos.pos() >= m->second.size())
      {
        XERR << "[角色技能-加载]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "快捷栏" << rCut.ShortDebugString() << "加载失败,栏位超过快捷栏最大上限" << m->second.size() << XEND;
        continue;
      }
      SPosData& rData = m->second[rPos.pos()];
      rData.dwID = rPos.id();
      rData.dwSourceid = rPos.sourceid();
    }
  }

  bool bNormalEmpty = true;
  bool bAutoEmpty = true;
  auto m = m_mapShortcut.find(ESKILLSHORTCUT_NORMAL);
  if (m != m_mapShortcut.end())
  {
    for (auto &v : m->second)
    {
      if (v.dwID != 0)
      {
        bNormalEmpty = false;
        break;
      }
    }
  }
  auto a = m_mapShortcut.find(ESKILLSHORTCUT_AUTO);
  if (a != m_mapShortcut.end())
  {
    for (auto &v : a->second)
    {
      if (v.dwID != 0)
      {
        bAutoEmpty = false;
        break;
      }
    }
  }
  if (bNormalEmpty && bAutoEmpty)
  {
    for (auto &data : m_vecSkillData)
    {
      for (auto &item : data.vecSkillItem)
      {
        for (auto &cut : item.mapShortcut)
          equipSkill(cut.first, item.dwID, item.dwSourceid, cut.second);
      }
    }
  }
  //m_vecPosSkill.clear();
  for (int i = 0; i < data.pos_size(); ++i)
  {
    const SkillPos& rPos = data.pos(i);
    const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(data.pos(i).id());
    if (pSkillCFG == nullptr || canEquip(pSkillCFG->getSkillType()) == false)
      continue;
    equipSkill(ESKILLSHORTCUT_NORMAL, rPos.id(), rPos.sourceid(), rPos.pos());
  }
  //m_vecAutoPosSkill.clear();
  for (int i = 0; i < data.autopos_size(); ++i)
  {
    const SkillPos& rPos = data.autopos(i);
    const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(data.autopos(i).id());
    if (pSkillCFG == nullptr || pSkillCFG->canEquipAuto() == false)
      continue;
    equipSkill(ESKILLSHORTCUT_AUTO, rPos.id(), rPos.sourceid(), rPos.pos());
  }
  //m_vecExtendPosSkill.clear();
  for (int i  = 0; i < data.extendpos_size(); ++i)
  {
    const SkillPos& rPos = data.extendpos(i);
    const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(data.extendpos(i).id());
    if (pSkillCFG == nullptr || canEquip(pSkillCFG->getSkillType()) == false)
      continue;
    equipSkill(ESKILLSHORTCUT_EXTEND, rPos.id(), rPos.sourceid(), rPos.pos());
  }
  // for old account
  /*if (data.pos_size() == 0 && data.autopos_size() == 0)
  {
    for (int i = 0; i < data.datas_size(); ++i)
    {
      for (int j = 0; j < data.datas(i).items_size(); ++j)
      {
        if (data.datas(i).items(j).pos() != 0 && data.datas(i).items(j).pos() <= m_dwMaxPos)
        {
          SPosData sdata;
          sdata.dwPos = data.datas(i).items(j).pos();
          sdata.dwID = data.datas(i).items(j).id();
          sdata.dwSourceid = data.datas(i).items(j).sourceid();
          m_vecPosSkill.push_back(sdata);
        }
        if (data.datas(i).items(j).autopos() != 0 && data.datas(i).items(j).autopos() <= m_dwAutoMaxPos)
        {
          SPosData sdata;
          sdata.dwPos = data.datas(i).items(j).autopos();
          sdata.dwID = data.datas(i).items(j).id();
          sdata.dwSourceid = data.datas(i).items(j).sourceid();
          m_vecAutoPosSkill.push_back(sdata);
        }
      }
    }
  }*/
  // 2016-05-13 , 技能ID 批量改动, 老账号登陆后重置所有技能, 并返还技能点
  if (data.reseted() == false)
  {
    resetSkill();
    /*for (auto v = m_vecSkillData.begin(); v != m_vecSkillData.end(); ++v)
    {
      if (v->eProfession == EPROFESSION_NOVICE)
        continue;

      m_dwTotalPoint += v->dwUsedPoint;
      v->dwUsedPoint = 0;
      v->vecSkillItem.clear();
    }*/
  }

  m_mapRecordReplaceID.clear();
  for (int i = 0; i < data.replace_size(); ++i)
  {
    DWORD oldid = data.replace(i).oldid();
    DWORD newid = data.replace(i).newid();
    m_mapRecordReplaceID[newid] = oldid;
    for (auto &v : m_vecSkillData)
    {
      auto s = find_if(v.vecSkillItem.begin(), v.vecSkillItem.end(), [&](const SSkillItem& r)->bool{
        return r.dwID == oldid;
      });
      if (s == v.vecSkillItem.end())
        continue;
      s->dwReplaceID = newid;
    }
  }

  m_dwLastConcertSkillID = data.last_concert_skillid();
  return true;
}

bool FighterSkill::save(UserSkillData* pData)
{
  if (pData == nullptr || m_pFighter == nullptr || m_pFighter->getUser() == nullptr)
  {
    XERR << "[角色技能-保存] 数据错误,必要指针都为空" << XEND;
    return false;
  }

  SceneUser* pUser = m_pFighter->getUser();

  pData->set_totalpoint(m_dwTotalPoint);
  pData->set_maxpos(m_dwMaxPos);
  pData->set_automaxpos(m_dwAutoMaxPos);
  pData->set_maxextendpos(m_dwMaxExtendPos);
  pData->set_reseted(true);

  pData->clear_datas();
  for (auto v = m_vecSkillData.begin(); v != m_vecSkillData.end(); ++v)
  {
    SkillData* data = pData->add_datas();
    if (data == nullptr)
    {
      XERR << "[角色技能-保存]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "create skillData protobuf error" << XEND;
      continue;
    }
    v->toData(data, m_pFighter);
  }

  pData->clear_shortcuts();
  for (auto &m : m_mapShortcut)
  {
    SkillShortcutDB* pCut = pData->add_shortcuts();
    pCut->set_type(m.first);
    for (auto &s : m.second)
    {
      if (s.dwID == 0 || s.dwPos == 0)
        continue;
      SkillPos* pPos = pCut->add_cuts();
      pPos->set_id(s.dwID);
      pPos->set_pos(s.dwPos);
      pPos->set_sourceid(s.dwSourceid);
    }
  }
  /*pData->clear_pos();
  for (auto &v : m_vecPosSkill)
  {
    SkillPos* pPos = pData->add_pos();
    if (pPos == nullptr)
    {
      XERR << "[角色技能-保存]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "create skillPos protobuf error" << XEND;
      continue;
    }
    pPos->set_pos(v.dwPos);
    pPos->set_id(v.dwID);
    pPos->set_sourceid(v.dwSourceid);
  }
  pData->clear_autopos();
  for (auto &v : m_vecAutoPosSkill)
  {
    SkillPos* pPos = pData->add_autopos();
    pPos->set_pos(v.dwPos);
    pPos->set_id(v.dwID);
    pPos->set_sourceid(v.dwSourceid);
  }
  pData->clear_extendpos();
  for (auto &v : m_vecExtendPosSkill)
  {
    SkillPos* pPos = pData->add_extendpos();
    pPos->set_pos(v.dwPos);
    pPos->set_id(v.dwID);
    pPos->set_sourceid(v.dwSourceid);
  }*/

  if (!m_mapRecordReplaceID.empty())
  {
    for (auto &m : m_mapRecordReplaceID)
    {
      SkillReplaceInfo* pInfo = pData->add_replace();
      if (pInfo)
      {
        pInfo->set_newid(m.first);
        pInfo->set_oldid(m.second);
      }
    }
  }

  pData->set_last_concert_skillid(m_dwLastConcertSkillID);

  XDBG << "[角色技能-保存]" << pUser->accid << pUser->id << m_pFighter->getProfession() << pUser->name << "数据大小:" << pData->ByteSize() << XEND;
  return true;
}

void FighterSkill::reload()
{
  for (auto v = m_vecSkillData.begin(); v != m_vecSkillData.end(); ++v)
  {
    for (auto o = v->vecSkillItem.begin(); o != v->vecSkillItem.end(); ++o)
      o->pSkillCFG = SkillManager::getMe().getSkillCFG(o->dwID);
  }

  if (m_pFighter != nullptr && m_pFighter->getUser() != nullptr)
  {
    SceneUser* pUser = m_pFighter->getUser();
    XLOG << "[角色技能-配置重加载]" << pUser->accid << pUser->id << m_pFighter->getProfession() << pUser->name << "更新了技能配置" << XEND;
  }
}

void FighterSkill::getSkillPosInfo(TVecPosSkill& vecSkillPosInfo)
{
  for (auto v = m_vecSkillData.begin(); v != m_vecSkillData.end(); ++v)
  {
    for (auto it = v->vecSkillItem.begin(); it != v->vecSkillItem.end(); ++it)
    {
      SPosData posData;
      posData.dwID = it->dwID;
      posData.dwSourceid = it->dwSourceid;
      posData.dwPos = 0;
      //if (getPos(it->dwID, it->dwSourceid))
      const SPosData* pData = getShortcut(ESKILLSHORTCUT_NORMAL, it->dwID, it->dwSourceid);
      if (pData != nullptr)
      {
        posData.dwPos = 1;                        //
        vecSkillPosInfo.push_back(posData);
      }
      //if (getAutoPos(it->dwID, it->dwSourceid))
      pData = getShortcut(ESKILLSHORTCUT_AUTO, it->dwID, it->dwSourceid);
      if (pData != nullptr)
      {
        SPosData posData;
        posData.dwID = it->dwID;
        posData.dwSourceid = it->dwSourceid;
        posData.dwPos = 2;                      //auto skill
        vecSkillPosInfo.push_back(posData);
      }
      if (posData.dwPos == 0)
      {
        vecSkillPosInfo.push_back(posData);
      }
    }
  }
}

bool FighterSkill::isSkillEnable(DWORD id)
{
  if (m_pFighter == nullptr)
    return false;
  SceneUser* pUser = m_pFighter->getUser();
  if (pUser == nullptr)
    return false;

  const SRoleBaseCFG* pCFG = m_pFighter->getRoleCFG();
  if (pCFG == nullptr)
    return false;
  if (id == pCFG->normalSkill || id == pCFG->strengthSkill)
    return true;

  const SNewRoleCFG& rCFG = MiscConfig::getMe().getNewRoleCFG();
  if (id == rCFG.dwCollectSkill || id == rCFG.dwTransSkill || id == rCFG.dwFlashSkill)// || id == rCFG.dwRepairSkill)
    return true;

  for (auto v = m_vecSkillData.begin(); v != m_vecSkillData.end(); ++v)
  {
    if ((*v).isSkillEnable(id))
      return true;
  }

  SceneFighter* pNoviceFighter = m_pFighter->getUser()->getFighter(EPROFESSION_NOVICE);
  if (pNoviceFighter != m_pFighter)
  {
    if (pNoviceFighter == nullptr)
      return false;
    if (pNoviceFighter->getSkill().isSkillEnable(id))
      return true;
  }

  return m_setTempIDs.find(id) != m_setTempIDs.end();
}

bool FighterSkill::checkSkill(DWORD dwSkillID)
{
  for (auto v = m_vecSkillData.begin(); v != m_vecSkillData.end(); ++v)
  {
    SSkillItem* pItem = v->getSkillByType(dwSkillID);
    if (pItem == nullptr)
      continue;
    if (!pItem->bLearn)
      continue;

    DWORD dwLv1 = getSkillLevel(dwSkillID);
    DWORD dwLv2 = getSkillLevel(pItem->dwID);
    if (dwLv2 >= dwLv1)
      return true;
  }

  SceneFighter* pNoviceFighter = m_pFighter->getUser()->getFighter(EPROFESSION_NOVICE);
  if (pNoviceFighter != m_pFighter && pNoviceFighter != nullptr)
  {
    if (pNoviceFighter->getSkill().checkSkill(dwSkillID))
      return true;
  }
  return false;
}

bool FighterSkill::canEquip(ESkillType eType) const
{
  static const vector<ESkillType> vecInvalid{ESKILLTYPE_PASSIVE, ESKILLTYPE_PURIFY};
  auto v = find(vecInvalid.begin(), vecInvalid.end(), eType);
  return v == vecInvalid.end();
}

void FighterSkill::sendData()
{
  // skill data
  sendSkillData();

  // optional skill
  SceneUser* pUser = m_pFighter->getUser();
  if (pUser)
  {
    const map<ESkillOption, DWORD>& skillopts = pUser->getUserSceneData().getSkillOpt();
    if (!skillopts.empty())
    {
      SkillOptionSkillCmd scmd;
      for (auto &m : skillopts)
      {
        SkillOption* p = scmd.add_all_opts();
        if (p == nullptr)
          continue;
        p->set_opt(m.first);
        p->set_value(m.second);
      }
      PROTOBUF(scmd, send, len);
      pUser->sendCmdToMe(send, len);
    }
  }

  // spec skill
  sendSpecSkillInfo();

  // valid pos
  sendValidPos();
}

void FighterSkill::sendSkillData()
{
  if (m_pFighter == nullptr)
    return;

  // skilldata
  ReqSkillData cmd;
  SceneUser* pUser = m_pFighter->getUser();
  if (pUser != nullptr && m_pFighter->getProfession() != EPROFESSION_NOVICE)
  {
    SceneFighter* pNoviceFighter = pUser->getFighter(EPROFESSION_NOVICE);
    if (pNoviceFighter != nullptr)
      //pNoviceFighter->getSkill().toClient(cmd);
    {
      SSkillData* pData = pNoviceFighter->getSkill().getSkillData(EPROFESSION_NOVICE);
      if (pData == nullptr)
        return;
      SkillData* data = cmd.add_data();
      if (data == nullptr)
        return;
      pData->toClientData(data, m_pFighter);
    }
  }

  for (auto v = m_vecSkillData.begin(); v != m_vecSkillData.end(); ++v)
  {
    if(!checkInBranch(v->eProfession))
      continue;

    SkillData* data = cmd.add_data();
    if (data == nullptr)
      continue;
    v->toClientData(data, m_pFighter);
  }
  PROTOBUF(cmd, send, len);
  m_pFighter->getUser()->sendCmdToMe(send, len);
}

void FighterSkill::sendValidPos()
{
  /*data.clear_pos();
  for (DWORD d = 1; d <= m_dwMaxPos; ++d)
    data.add_pos(d);
  data.clear_autopos();
  for (DWORD d = 1; d <= m_dwAutoMaxPos; ++d)
    data.add_autopos(d);
  for (DWORD d = 1; d <= m_dwMaxExtendPos; ++d)
    data.add_extendpos(d);

  if (m_bMonthCardSkillValid)
    data.add_autopos(MONTH_CARD_SKILL_POS);*/

  SceneUser* pUser = m_pFighter->getUser();
  if (pUser == nullptr)
    return;

  SkillValidPos data;
  toData(&data);

  PROTOBUF(data, send, len);
  pUser->sendCmdToMe(send, len);
}

void FighterSkill::toData(SkillValidPos* pData)
{
  if (pData == nullptr || m_pFighter == nullptr || m_pFighter->getUser() == nullptr)
    return;
  pData->Clear();

  SceneUser* pUser = m_pFighter->getUser();
  SkillValidShortcut* pCut = pData->add_shortcuts();
  pCut->set_type(ESKILLSHORTCUT_NORMAL);
  for (DWORD d = 1; d <= m_dwMaxPos; ++d)
    pCut->add_pos(d);

  pCut = pData->add_shortcuts();
  pCut->set_type(ESKILLSHORTCUT_AUTO);
  for (DWORD d = 1; d <= m_dwAutoMaxPos; ++d)
    pCut->add_pos(d);
  if (m_bMonthCardSkillValid)
    pCut->add_pos(MONTH_CARD_SKILL_POS);

  pCut = nullptr;
  for (DWORD d = 1; d <= m_dwMaxExtendPos; ++d)
  {
    DWORD pos = (d - 1) % SKILL_MAX_CUTS;
    DWORD type = (d - 1) / SKILL_MAX_CUTS;
    if (type >= VEC_SHORTCUT_EXTEND.size())
    {
      XERR << "[角色技能-合法快捷栏]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "采集 type :" << type << "pos :" << pos << "失败,超过合法扩展栏上限" << VEC_SHORTCUT_EXTEND.size() << XEND;
      continue;
    }
    if (pos == 0)
    {
      pCut = pData->add_shortcuts();
      pCut->set_type(VEC_SHORTCUT_EXTEND[type]);
    }
    if (pCut == nullptr)
    {
      XERR << "[角色技能-合法快捷栏]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "采集 type :" << type << "pos :" << pos << "失败,未正确初始化" << XEND;
      continue;
    }
    pCut->add_pos(pos + 1);
  }
  XDBG << "[角色技能-合法快捷栏]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "采集有效技能快捷栏" << pData->ShortDebugString() << XEND;
}

void FighterSkill::setSkillPoint(DWORD point, ESource eSource)
{
  if (m_pFighter == nullptr)
    return;

  if (m_pFighter->getProfession() == EPROFESSION_NOVICE)
    return;

  SceneUser* pUser = m_pFighter->getUser();
  if (pUser == nullptr)
    return;

  if (eSource == ESOURCE_LVUP && point > m_dwTotalPoint && m_pFighter != nullptr && m_pFighter->getUser() != nullptr)
    MsgManager::sendMsg(m_pFighter->getUser()->id, 43, MsgParams(1));

  XLOG << "[角色技能-技能点设置]" << pUser->accid << pUser->id << m_pFighter->getProfession() << pUser->name << "pro=" << m_pFighter->getProfession() << "oldpoint =" << m_dwTotalPoint << "curpoint =" << point << XEND;

  m_dwTotalPoint = point;
  m_pFighter->getUser()->setDataMark(EUSERDATATYPE_SKILL_POINT);
  m_pFighter->getUser()->getEvent().onSkillPointChange();
}

DWORD FighterSkill::getUsedPoint() const
{
  DWORD dwPoint = 0;
  for (auto v = m_vecSkillData.begin(); v != m_vecSkillData.end(); ++v)
    dwPoint += v->dwUsedPoint;

  return dwPoint;
}

DWORD FighterSkill::getSkillBTPoint()
{
  DWORD dwbt = 0;
  for (auto v = m_vecSkillData.begin(); v != m_vecSkillData.end(); ++v)
  {
    if (v->eProfession == EPROFESSION_NOVICE)
      continue;

    for (auto o = v->vecSkillItem.begin(); o != v->vecSkillItem.end(); ++o)
    {
      if (!o->bLearn)
        continue;
      dwbt += getSkillLevel(o->dwID);
    }
  }

  return dwbt;
}

void FighterSkill::setMaxPos(DWORD dwPos)
{
  if (m_dwMaxPos == dwPos)
    return;
  m_dwMaxPos = dwPos;

  // inform skill pos
  /*SkillValidPos cmd;
  toClient(cmd);
  PROTOBUF(cmd, send, len);
  m_pFighter->getUser()->sendCmdToMe(send, len);*/
  sendValidPos();
}

void FighterSkill::setAutoMaxPos(DWORD dwPos)
{
  if (m_pFighter == nullptr || m_pFighter->getUser() == nullptr)
    return;

  if (m_dwAutoMaxPos == dwPos)
    return;
  m_dwAutoMaxPos = dwPos;

  /*SkillValidPos cmd;
  toClient(cmd);
  PROTOBUF(cmd, send, len);
  m_pFighter->getUser()->sendCmdToMe(send, len);*/
  sendValidPos();

  // 自动槽, 默认指定技能

  SceneFighter* pNoviceFighter = m_pFighter->getUser()->getFighter(EPROFESSION_NOVICE);
  if (pNoviceFighter == nullptr)
    return;
  if (m_dwAutoMaxPos == 2 && isSkillEnable(11001))// && getAutoPos(11001, 0) != 1)
  {
    SPosData* pData = getShortcut(ESKILLSHORTCUT_AUTO, 11001, 0);
    if (pData != nullptr)
    {
      if (pData->dwPos != 1)
      {
        pData->clear();
        equipSkill(ESKILLSHORTCUT_AUTO, 11001, 0, 2);
      }
    }
    //setAutoPos(11001, 0, 2);
    addupdate(EPROFESSION_NOVICE, 11001, 0);
  }
}

void FighterSkill::setMaxExtendPos(DWORD dwPos)
{
  if (m_dwMaxExtendPos == dwPos)
    return;
  m_dwMaxExtendPos = dwPos;

  /*SkillValidPos cmd;
  toClient(cmd);
  PROTOBUF(cmd, send, len);
  m_pFighter->getUser()->sendCmdToMe(send, len);*/
  sendValidPos();
}

void FighterSkill::decMaxPos()
{
  if (m_dwMaxPos == 0)
    return;

  /*auto it = find_if(m_vecPosSkill.begin(), m_vecPosSkill.end(), [&](const SPosData&r) ->bool{
     return r.dwPos == m_dwMaxPos;
      });
  if (it != m_vecPosSkill.end())
  {
    addPosUpdate(it->dwID, it->dwSourceid);
    m_vecPosSkill.erase(it);
  }*/
  SPosData* pData = getShortcut(ESKILLSHORTCUT_NORMAL, m_dwMaxPos);
  if (pData != nullptr && pData->dwID != 0)
  {
    addPosUpdate(pData->dwID, pData->dwSourceid);
    pData->clear();
  }
  --m_dwMaxPos;

  if (!m_pFighter)
    return;
  SceneUser* user = m_pFighter->getUser();
  if (!user)
    return;

  /*SkillValidPos cmd;
  toClient(cmd);
  PROTOBUF(cmd, send, len);
  user->sendCmdToMe(send, len);*/
  sendValidPos();

  XLOG << "[技能格子-减1], 玩家:" << user->name << user->id << "当前格子数量:" << m_dwMaxPos << XEND;
}

void FighterSkill::decAutoMaxPos()
{
  if (m_dwAutoMaxPos == 0)
    return;

  /*auto it = find_if(m_vecAutoPosSkill.begin(), m_vecAutoPosSkill.end(), [&](const SPosData& r) ->bool{
      return r.dwPos == m_dwAutoMaxPos;
      });
  if (it != m_vecAutoPosSkill.end())
  {
    addPosUpdate(it->dwID, it->dwSourceid);
    m_vecAutoPosSkill.erase(it);
  }*/
  SPosData* pData = getShortcut(ESKILLSHORTCUT_AUTO, m_dwAutoMaxPos);
  if (pData != nullptr && pData->dwID != 0)
  {
    addPosUpdate(pData->dwID, pData->dwSourceid);
    pData->clear();
  }

  --m_dwAutoMaxPos;

  if (!m_pFighter)
    return;
  SceneUser* user = m_pFighter->getUser();
  if (!user)
    return;

  /*SkillValidPos cmd;
  toClient(cmd);
  PROTOBUF(cmd, send, len);
  user->sendCmdToMe(send, len);*/
  sendValidPos();

  XLOG << "[技能自动格子-减1], 玩家:" << user->name << user->id << "当前格子数量:" << m_dwAutoMaxPos << XEND;
}

void FighterSkill::decMaxExtendPos()
{
  if (m_dwMaxExtendPos == 0)
    return;

  /*auto it = find_if(m_vecExtendPosSkill.begin(), m_vecExtendPosSkill.end(), [&](const SPosData&r) ->bool{
     return r.dwPos == m_dwMaxExtendPos;
      });
  if (it != m_vecExtendPosSkill.end())
  {
    addPosUpdate(it->dwID, it->dwSourceid);
    m_vecExtendPosSkill.erase(it);
  }*/
  DWORD dwNum = (m_dwMaxExtendPos - 1) / SKILL_MAX_CUTS;
  DWORD dwPos = (m_dwMaxExtendPos - 1 ) % SKILL_MAX_CUTS + 1;
  SPosData* pData = getShortcut(VEC_SHORTCUT_EXTEND[dwNum], dwPos);
  if (pData != nullptr && pData->dwID != 0)
  {
    addPosUpdate(pData->dwID, pData->dwSourceid);
    pData->clear();
  }
  --m_dwMaxExtendPos;

  if (!m_pFighter)
    return;
  SceneUser* user = m_pFighter->getUser();
  if (!user)
    return;

  /*SkillValidPos cmd;
  toClient(cmd);
  PROTOBUF(cmd, send, len);
  user->sendCmdToMe(send, len);*/
  sendValidPos();

  XLOG << "[技能格子-减1], 玩家:" << user->name << user->id << "当前格子数量:" << m_dwMaxExtendPos << XEND;
}

// 转职时调用一次
bool FighterSkill::addEquipSkill(FighterSkill& oldFs)
{
  const SSkillData* pData = oldFs.getSkillData(EPROFESSION_NOVICE);
  if (pData == nullptr)
    return false;

  for (auto v = pData->vecSkillItem.begin(); v != pData->vecSkillItem.end(); ++v)
  {
    if ((*v).dwSourceid == 0 || (*v).eSource == ESOURCE_MIN)
      continue;
    addSkill((*v).dwID, (*v).dwSourceid, (*v).eSource);
  }
  update(now());
  return true;
}

void FighterSkill::getEquipSkill(DWORD oneid, TVecDWORD& vecids)
{
  for (auto &v : m_vecSkillData)
  {
    for (auto & s : v.vecSkillItem)
    {
      if (s.dwID/100 == oneid/100)
        vecids.push_back(s.dwID);
    }
  }
}

DWORD FighterSkill::getNormalSkill()
{
  if (m_pFighter == nullptr)
    return 0;
  SceneUser* pUser = m_pFighter->getUser();
  if (pUser == nullptr)
    return 0;
  if (pUser->getTransform().isMonster())
  {
    return pUser->getTransform().getNormalSkillID();
  }
  if (m_dwReplaceNormalSkillID)
    return m_dwReplaceNormalSkillID;
  const SRoleBaseCFG* pCFG = pUser->getRoleBaseCFG();
  if (pCFG == nullptr)
    return 0;
  return pCFG->normalSkill;
}

void FighterSkill::refreshEnableSkill()
{
  if (m_pFighter == nullptr || m_pFighter->getUser() == nullptr)// || m_pFighter->getUser()->getRoleBaseCFG() == nullptr)
    return;

  SceneFighter* pNoviceFighter = m_pFighter->getUser()->getFighter(EPROFESSION_NOVICE);
  if (pNoviceFighter != nullptr && pNoviceFighter != m_pFighter)
  {
    pNoviceFighter->getSkill().refreshEnableSkill();
    pNoviceFighter->getSkill().update(now());
  }

  const SRoleBaseCFG* pCFG = m_pFighter->getRoleCFG();
  if (pCFG == nullptr)
    return;
  SceneUser* pUser = m_pFighter->getUser();

  // check enable skill/
  for (auto v = pCFG->vecEnableSkill.begin(); v != pCFG->vecEnableSkill.end(); ++v)
  {
    SSkillData* pData = getSkillData(v->first);
    if (pData == nullptr)
    {
      XERR << "[角色技能-刷新]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "未找到" << v->first << "技能" << XEND;
      continue;
    }

    for (auto o = v->second.begin(); o != v->second.end(); ++o)
    {
      /*if (m_pFighter->getUser()->getUserSceneData().getProfession() != EPROFESSION_NOVICE)
      {
        TVecDWORD vecTempinvalid{12001, 229001};
        auto s = find(vecTempinvalid.begin(), vecTempinvalid.end(), *o);
        if (s != vecTempinvalid.end())
          continue;
      }*/

      // get skill and add if noexist
      SSkillItem* pItem = pData->getSkillByType(*o);
      if (pItem == nullptr)
      {
        const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(*o);
        if (pSkillCFG == nullptr)
        {
          XERR << "[角色技能-刷新]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "未找到 pro:" << v->first << "skillid:" << *o << "技能" << XEND;
          continue;
        }

        // check condition if novice
        if (v->first == EPROFESSION_NOVICE && pSkillCFG->checkCondition(m_pFighter->getUser()) == false)
            continue;

        SSkillItem stItem;
        stItem.dwID = *o;
        stItem.eProfession = v->first;
        stItem.pSkillCFG = pSkillCFG;
        stItem.consume.nexttime = now();
        pData->vecSkillItem.push_back(stItem);
        addupdate(v->first, *o);
        pItem = pData->getSkillItem(*o);
      }
      if (pItem == nullptr || pItem->pSkillCFG == nullptr)
        continue;
      if (pItem->eProfession == EPROFESSION_NOVICE)
      {
        pItem->bLearn = true;
        pItem->bActive = false;

        const BaseSkill* pSkillCFG = pItem->pSkillCFG;
        if (pSkillCFG->getSkillType() != ESKILLTYPE_PASSIVE)
        {
          if (m_pFighter->getUser()->getProfession() == EPROFESSION_NOVICE)
          {
            //setValidPos(*o, 0);
            equipSkill(ESKILLSHORTCUT_NORMAL, *o, 0);
            if (pSkillCFG->canEquipAuto() && pCFG->normalSkill == *o)
              //setValidAutoPos(*o, 0);
              equipSkill(ESKILLSHORTCUT_AUTO, *o, 0);
          }
        }
        if (pSkillCFG->haveDirectBuff())
          m_pFighter->getUser()->m_oBuff.addSkillBuff(*o);
      }

      // check active
      if (pItem->bActive || pItem->eProfession == EPROFESSION_NOVICE)
        continue;
      if (pItem->pSkillCFG == nullptr)
      {
        XERR << "[角色技能-刷新]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "pro:" << pItem->eProfession << "skillid:" << *o << "配置未初始化" << XEND;
        continue;
      }
      DWORD dwSkillID = pItem->pSkillCFG->getNextSkillID();
      if(dwSkillID == 0 && m_pFighter->getUser()->getMenu().isOpen(EMENUID_PEAK_LEVEL) == true && MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_PEAK) == false)
        dwSkillID = pItem->pSkillCFG->getBreakSkillID();
      if (dwSkillID == 0 && m_pFighter->getUser()->getMenu().isOpen(EMENUID_DEAD) == true && MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_DEAD) == false)
      {
        if (m_pFighter->getUser()->getUserSceneData().getDeadLv() >= pItem->pSkillCFG->getDeadLvReq())
          dwSkillID = pItem->pSkillCFG->getDeadSkillID();
      }
      const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(dwSkillID);
      if (pSkillCFG != nullptr && pSkillCFG->checkCondition(m_pFighter->getUser()) == true)
      {
        pItem->bActive = true;
        addupdate(v->first, pSkillCFG->getSkillID());
      }
    }
  }
}

bool FighterSkill::addSkill(DWORD skillid, DWORD sourceid, ESource eSource, bool needpos, bool bNotify /* = true*/)
{
  if (m_pFighter == nullptr || m_pFighter->getUser() == nullptr)
    return false;

  const SRoleBaseCFG* pNoviceCFG = RoleConfig::getMe().getRoleBase(EPROFESSION_NOVICE);
  if (pNoviceCFG == nullptr)
    return false;

  const SRoleBaseCFG* pNowCFG = RoleConfig::getMe().getRoleBase(m_pFighter->getProfession());
  if (pNowCFG == nullptr)
    return false;

  SceneFighter* pNoviceFighter = m_pFighter->getUser()->getFighter(EPROFESSION_NOVICE);
  if (pNoviceFighter == nullptr)
    return false;

  SSkillData* pData = pNoviceFighter->getSkill().getSkillData(EPROFESSION_NOVICE);
  if (pData == nullptr)
    return false;

  const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(skillid);
  if (pSkillCFG == nullptr)
    return false;

  SSkillItem* pItem = pData->getSkillItem(skillid, sourceid, eSource);
  if (pItem != nullptr)
  {
    if (pItem->bShadow == false)
      return false;
    pItem->bShadow = false;
    addupdate(EPROFESSION_NOVICE, skillid, sourceid);

    if (pSkillCFG->getSkillType() == ESKILLTYPE_ENSEMBLE)
      m_pFighter->getUser()->updateEnsembleSkill();
    return true;
  }

  SSkillItem stItem;
  stItem.dwID = skillid;
  stItem.eSource = eSource;
  stItem.eProfession = EPROFESSION_NOVICE;
  stItem.pSkillCFG = pSkillCFG;
  stItem.dwSourceid = sourceid;
  stItem.consume.nexttime = now();
  stItem.bLearn = true;
  stItem.bActive = false;
  stItem.qwOwnerID = m_pFighter->getUser()->id;
  if (stItem.pSkillCFG->haveDirectBuff())
  {
    m_pFighter->getUser()->m_oBuff.addSkillBuff(skillid);
  }

  // update skill func
  if (skillid == MiscConfig::getMe().getExpressionCFG().dwBlinkNeedSkill)
    m_pFighter->getUser()->getUserSceneData().setBlink(true);
  else if (MiscConfig::getMe().getItemCFG().isPackSkill(skillid) == true)
  {
    m_pFighter->getUser()->getUserSceneData().insertPackSkill(skillid);
    m_pFighter->getUser()->getPackage().refreshMaxSlot();
  }
  m_pFighter->getUser()->getEvent().onAddSkill(skillid);

  /*if (pNoviceCFG->normalSkill == skillid || pNoviceCFG->strengthSkill == skillid)
  {
    stItem.bLearn = true;
    stItem.bActive = true;
    stItem.dwPos = getValidPos();
  }
  if (ESOURCE_EQUIP == eSource || ESOURCE_CARD == eSource)
  {
    stItem.bLearn = true;
    stItem.bActive = false;
  }*/
  pData->vecSkillItem.push_back(stItem);
  addupdate(EPROFESSION_NOVICE, skillid, sourceid);

  if (stItem.pSkillCFG->getSkillType() != ESKILLTYPE_PASSIVE)
  {
    if (needpos && sourceid == 0)
    {
      /*if (getPos(skillid, sourceid) == 0)
        setValidPos(skillid, sourceid);
      if (skillid == pNowCFG->normalSkill && pSkillCFG->canEquipAuto() && getAutoPos(skillid, sourceid) == 0)
        setValidAutoPos(skillid, sourceid);*/
      SPosData* pData = getShortcut(ESKILLSHORTCUT_NORMAL, skillid, sourceid);
      if (pData == nullptr)
        equipSkill(ESKILLSHORTCUT_NORMAL, skillid, sourceid);
      if (skillid == getNormalSkill() && pSkillCFG->canEquipAuto())
      {
        pData = getShortcut(ESKILLSHORTCUT_AUTO, skillid, sourceid);
        if (pData == nullptr)
          equipSkill(ESKILLSHORTCUT_AUTO, skillid, sourceid);
      }
    }
  }

  // 立刻更新
  update(now());
  // 更新menu
  if (eSource == ESOURCE_MIN || eSource == ESOURCE_SHOP || eSource == ESOURCE_NORMAL)
    m_pFighter->getUser()->getMenu().refreshNewMenu(EMENUCOND_SKILL);
  if (eSource == ESOURCE_SHOP || eSource == ESOURCE_NORMAL)
  {
    if(bNotify)
      MsgManager::sendMsg(m_pFighter->getUser()->id, 24, MsgParams(pSkillCFG->getName()));
  }

  if (pSkillCFG->getSkillType() == ESKILLTYPE_ENSEMBLE)
    m_pFighter->getUser()->updateEnsembleSkill();

  if (eSource == ESOURCE_SHOP || eSource == ESOURCE_NORMAL)
  {
    //log
    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_Manual_Skill;
    PlatLogManager::getMe().eventLog(thisServer,
      m_pFighter->getUser()->getUserSceneData().getPlatformId(),
      m_pFighter->getUser()->getZoneID(),
      m_pFighter->getUser()->accid,
      m_pFighter->getUser()->id,
      eid,
      m_pFighter->getUser()->getUserSceneData().getCharge(), eType, 0, 1);

    PlatLogManager::getMe().ManualLog(thisServer,
      m_pFighter->getUser()->getUserSceneData().getPlatformId(),
      m_pFighter->getUser()->getZoneID(),
      m_pFighter->getUser()->accid,
      m_pFighter->getUser()->id,
      eType,
      eid,/*eid*/
      EManual_Skill,
      0,
      sourceid,
      skillid);
  }
  return true;
}

bool FighterSkill::removeSkill(DWORD skillid, DWORD sourceid, ESource eSource /*= ESOURCE_EQUIP*/, bool onlyCheckID /*=false*/)
{
  if (m_pFighter == nullptr || m_pFighter->getUser() == nullptr)
    return false;
  SceneFighter* pNoviceFighter = m_pFighter->getUser()->getFighter(EPROFESSION_NOVICE);
  if (pNoviceFighter == nullptr)
    return false;

  SSkillData* pData = pNoviceFighter->getSkill().getSkillData(EPROFESSION_NOVICE);
  if (pData == nullptr)
    return false;

  auto v = find_if(pData->vecSkillItem.begin(), pData->vecSkillItem.end(), [skillid, eSource, sourceid, onlyCheckID](const SSkillItem& r) -> bool{
    if (onlyCheckID == false)
      return r.dwID == skillid && r.dwSourceid == sourceid && r.eSource == eSource;
    else
      return r.dwID == skillid;
  });
  if (v == pData->vecSkillItem.end())
    return false;

  // 共享技能, 不属于自己添加的不可移除
  const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(v->dwID);
  if (pSkillCFG && !onlyCheckID)
  {
    if (pSkillCFG->isShareSkill() && v->qwOwnerID != m_pFighter->getUser()->id)
      return false;
  }

  m_pFighter->getUser()->m_oBuff.delSkillBuff(skillid);
  m_pFighter->getUser()->getEvent().onDelSkill(skillid);

  bool bShop = (v->eSource == ESOURCE_SHOP || v->eSource == ESOURCE_NORMAL);
  v->bShadow = true;
  if (isInSlot(v->dwID, v->dwSourceid) == false)
    pData->vecSkillItem.erase(v);
  else if (v->dwSourceid == 0)
  {
    /*setPos(v->dwID, v->dwSourceid, 0);
    setAutoPos(v->dwID, v->dwSourceid, 0);
    setExtendPos(v->dwID, v->dwSourceid, 0);*/
    removeShortcut(ESKILLSHORTCUT_MIN, v->dwID, v->dwSourceid);
    pData->vecSkillItem.erase(v);
  }
  addupdate(EPROFESSION_NOVICE, skillid, sourceid);

  if (bShop)
  {
    m_pFighter->getUser()->getMenu().checkBackoffSkillMenu();
    m_pFighter->getUser()->getPackage().checkInvalidEquip(true);
  }

  if (pSkillCFG && pSkillCFG->getSkillType() == ESKILLTYPE_ENSEMBLE)
    m_pFighter->getUser()->updateEnsembleSkill();

  return true;
}

bool FighterSkill::resetShopSkill()
{
  if (m_pFighter == nullptr)
    return false;
  SceneUser* user = m_pFighter->getUser();
  if (user == nullptr)
    return false;
  SceneFighter* pNoviceFighter = user->getFighter(EPROFESSION_NOVICE);
  if (pNoviceFighter == nullptr)
    return false;
  SSkillData* pData = pNoviceFighter->getSkill().getSkillData(EPROFESSION_NOVICE);
  if (pData == nullptr)
    return false;

  bool bFind = false;
  DWORD addTotalPoint = 0, addTotalZeny = 0;
  const SItemMiscCFG& rCFG = MiscConfig::getMe().getItemCFG();
  const SFoodMiscCFG& rFoodCFG = MiscConfig::getMe().getFoodCfg();
  const SWeddingMiscCFG& rWeddingCFG = MiscConfig::getMe().getWeddingMiscCFG();
  const SPetMiscCFG& rPetCFG = MiscConfig::getMe().getPetCFG();
  for (auto v = pData->vecSkillItem.begin(); v != pData->vecSkillItem.end(); )
  {
    if (v->eSource == ESOURCE_SHOP)
    {
      const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(v->dwID);
      if (pSkill == nullptr)
      {
        ++v;
        continue;
      }
      // 共享技能, 不属于自己添加的不可移除
      if (pSkill->isShareSkill() && v->qwOwnerID != user->id)
      {
        ++v;
        continue;
      }
      if (rFoodCFG.dwSkillId == v->dwID)
      {
        ++v;
        continue;
      }
      auto s = rWeddingCFG.setMarrySkills.find(v->dwID);
      if (s != rWeddingCFG.setMarrySkills.end())
      {
        ++v;
        continue;
      }
      auto m = rPetCFG.mapSkillWork.find(v->dwID);
      if (m != rPetCFG.mapSkillWork.end())
      {
        ++v;
        continue;
      }

      if(bFind == false)
        MsgManager::sendMsg(user->id, 4002);

      DWORD shopskill = v->dwID;
      DWORD addpoint = pSkill->getLevelCost();
      addupdate(EPROFESSION_NOVICE, v->dwID, v->dwSourceid);
      addTotalPoint += addpoint;
      v = pData->vecSkillItem.erase(v);
      //user->getManual().addSkillPoint(addpoint);
      // get shop config
      const ShopItem* pItem = ShopConfig::getMe().getShopItemBySkill(shopskill);
      if(pItem != nullptr)
      {
        ItemInfo stInfo;
        stInfo.set_id(pItem->moneyid());
        stInfo.set_count(pItem->moneycount());
        stInfo.set_source(ESOURCE_SHOP);
        /*BasePackage* pMainPack = user->getPackage().getPackage(EPACKTYPE_MAIN);
        if(pMainPack != nullptr)
          pMainPack->addItemFull(stInfo);*/
        user->getPackage().addItem(stInfo, EPACKMETHOD_AVAILABLE);

        stInfo.set_id(pItem->moneyid2());
        stInfo.set_count(pItem->moneycount2());
        stInfo.set_source(ESOURCE_SHOP);
        /*if(pMainPack != nullptr)
          pMainPack->addItemFull(stInfo);*/
        user->getPackage().addItem(stInfo, EPACKMETHOD_AVAILABLE);
        DWORD addZeny = pItem->moneycount() + pItem->moneycount2();
        addTotalZeny += addZeny;
        MsgManager::sendMsg(user->id, 4003, MsgParams(pSkill->getName(), addpoint, addZeny));
      }

      if (rCFG.setPackSkill.find(pSkill->getSkillID()) != rCFG.setPackSkill.end())
        user->getUserSceneData().erasePackSkill(pSkill->getSkillID());

      bFind = true;
      user->getEvent().onDelSkill(pSkill->getSkillID());
      XLOG << "[冒险技能-重置], 玩家:" << user->name << user->id << "删除冒险技能:" << pSkill->getName() << "返还冒险技能点:" << pSkill->getLevelCost() << XEND;
      continue;
    }
    ++v;
  }

  user->getManual().calcSkillPoint(true);
  user->getPackage().refreshMaxSlot();
  user->setCollectMark(ECOLLECTTYPE_DYNAMIC_BUFF);
  user->refreshDataAtonce();
  if(addTotalPoint != 0 || addTotalZeny != 0)
    MsgManager::sendMsg(user->id, 4004, MsgParams(addTotalPoint, addTotalZeny));

  user->getPackage().checkInvalidEquip(true);
  user->getMenu().checkBackoffSkillMenu();
  return bFind;
}

bool FighterSkill::checkSkillPointIllegal()
{
  if (m_pFighter == nullptr)
    return false;
  SceneUser* pUser = m_pFighter->getUser();
  if (pUser == nullptr)
    return false;
  TVecDWORD rankPoints;
  for (auto &v : m_vecSkillData)
  {
    if (v.eProfession == EPROFESSION_NOVICE)
      continue;
    rankPoints.push_back(v.dwUsedPoint);
  }
  DWORD size = rankPoints.size();
  if (size >= 2)
  {
    TVecDWORD vecAddPoints;
    for (auto &v : rankPoints) vecAddPoints.push_back(v);
    for (DWORD i = 0; i < size - 1; ++i)
    {
      vecAddPoints[i+1] += vecAddPoints[i];
    }

    for (DWORD i = 0; i < size - 1; ++i)
    {
      if (rankPoints[i+1] != 0)
      {
        if (vecAddPoints[i] < (i + 1) * ONE_JOB_POINT)
        {
          XERR << "[技能加点-检查], 前置技能不满足" << pUser->name << pUser->id << m_pFighter->getProfession() << "前置技能点数:" << vecAddPoints[i] << XEND;
          return true;
        }
      }
    }
  }
  return false;
}

bool FighterSkill::levelupSkill(const LevelupSkill& cmd)
{
  if (cmd.skillids_size() <= 0)
    return false;

  SceneUser* pUser = m_pFighter->getUser();
  if (pUser == nullptr)
    return false;
  // level up data
  vector<tuple<SSkillItem, SSkillItem*, DWORD>> vecSkills;
  vector<pair<EProfession, DWORD>> vecPoints;
  DWORD dwNeedPoint = 0;
  DWORD dwUsedPoint = 0;
  DWORD dwUsedPrimaryPoint = 0;
  bool blBreak = false;
  map<EProfession, DWORD> mapPrimaryPoint;
  map<EProfession, pair<DWORD, DWORD>> mapPro2Point; // 本次升级技能后, 每个职业对应已消耗的点数和本次需要消耗的点数

  // collect usepoint
  auto point = [&](EProfession eProfession, DWORD usePoint, bool primary) -> bool
  {
    dwNeedPoint += usePoint;
    auto it = mapPro2Point.find(eProfession);
    if (it == mapPro2Point.end())
    {
      mapPro2Point[eProfession].first = 0;
      mapPro2Point[eProfession].second = usePoint;
    }
    else
      it->second.second += usePoint;
    auto v = find_if(vecPoints.begin(), vecPoints.end(), [eProfession](const pair<EProfession, DWORD>& r) -> bool{
      return r.first == eProfession;
    });
    if (v == vecPoints.end())
    {
      vecPoints.push_back(pair<EProfession, DWORD>(eProfession, 0));
      v = find_if(vecPoints.begin(), vecPoints.end(), [eProfession](const pair<EProfession, DWORD>& r) -> bool{
          return r.first == eProfession;
      });
    }
    if (v == vecPoints.end())
      return false;
    if(primary == true)
      mapPrimaryPoint[eProfession] += usePoint;

    v->second += usePoint;
    return true;
  };

  // recover data
  auto recover = [&]()
  {
    for (auto v = vecSkills.begin(); v != vecSkills.end(); ++v)
      *get<1>(*v) = get<0>(*v);
  };

  // check and collect data
  for (auto v = m_vecSkillData.begin(); v != m_vecSkillData.end(); ++v)
  {
    dwUsedPrimaryPoint += v->dwPrimaryPoint;
    dwUsedPoint += v->dwUsedPoint;
    mapPro2Point[v->eProfession].first = v->dwUsedPoint;
    mapPro2Point[v->eProfession].second = 0;
    for (int i = 0; i < cmd.skillids_size(); ++i)
    {
      SSkillItem* pItem = v->getSkillByType(cmd.skillids(i));
      if (pItem != nullptr)
      {
        if (pItem->dwID > cmd.skillids(i))
          return false;
        if (pItem->dwID == cmd.skillids(i) && pItem->bLearn)
          return false;
        if (pItem->pSkillCFG == nullptr)
          return false;

        vecSkills.push_back(make_tuple(*pItem, pItem, cmd.skillids(i)));

        if (!pItem->bLearn)
          point(v->eProfession, pItem->pSkillCFG->getLevelCost(), true);

        DWORD dwSkillID = pItem->pSkillCFG->getSkillID();
        const BaseSkill* pSkillCFG = pItem->pSkillCFG;
        while (dwSkillID != cmd.skillids(i))
        {
          dwSkillID = pSkillCFG->getNextSkillID();
          bool blSystemForbide = MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_PEAK);
          bool blFunctionOpen = m_pFighter->getUser()->getMenu().isOpen(EMENUID_PEAK_LEVEL);
          //bool blBreakPoint = (m_pFighter->getJobLv() >= m_dwTotalPoint) && ((m_pFighter->getJobLv() - m_dwTotalPoint) >= MiscConfig::getMe().getSkillBreakPoint());
          bool blPrimary = true;
          if(dwSkillID == 0)
          {
            if(blSystemForbide == false && blFunctionOpen == true)
            {
              dwSkillID = pSkillCFG->getBreakSkillID();
              blPrimary = false;
              blBreak = true;
            }
          }
          if (dwSkillID == 0)
          {
            blSystemForbide = MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_DEAD);
            blFunctionOpen = m_pFighter->getUser()->getMenu().isOpen(EMENUID_DEAD);
            if (blSystemForbide == false && blFunctionOpen == true)
            {
              if (m_pFighter->getUser()->getUserSceneData().getDeadLv() < pSkillCFG->getDeadLvReq())
              {
                SceneUser* pUser = m_pFighter->getUser();
                MsgManager::sendDebugMsg(pUser->id, "测试log:亡者气息等级不足");
                XERR << "[角色技能-升级]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "升级技能skillid:" << cmd.skillids(i) << "失败,亡者气息等级不足" << XEND;
                return false;
              }
              dwSkillID = pSkillCFG->getDeadSkillID();
            }
          }

          pSkillCFG = SkillManager::getMe().getSkillCFG(dwSkillID);
          if (pSkillCFG == nullptr)
            return false;
          point(v->eProfession, pSkillCFG->getLevelCost(), blPrimary);
        }
      }
    }
  }
  if (vecSkills.empty() == true || vecPoints.empty() == true)
    return false;

  // check primarypoint
  for(auto s : mapPrimaryPoint)
    dwUsedPrimaryPoint += s.second;
  // check point
  bool blTotal = false;
  if(dwUsedPoint < MiscConfig::getMe().getSkillBreakPoint() && (dwUsedPoint + dwNeedPoint > MiscConfig::getMe().getSkillBreakPoint()))
    blTotal = true;
  if (m_dwTotalPoint < dwNeedPoint || (blBreak == true && dwUsedPoint < MiscConfig::getMe().getSkillBreakPoint()) || blTotal == true)
  {
    recover();
    return false;
  }

  // 某几转的技能要求前面几转的技能点必须消耗掉一定数量才可以升级
  DWORD dwPreUsedPoint = 0;
  for (auto& v : mapPro2Point)
  {
    if (v.second.second > 0 && dwPreUsedPoint < MiscConfig::getMe().getPeakLevelCFG().getNeedPreSkillPoint(v.first))
    {
      recover();
      return false;
    }
    dwPreUsedPoint += v.second.first + v.second.second;
  }

  // 先排序 再计算
  // 虽然恶心 但没好办法
  std::map<DWORD, DWORD> mapProPoint; // <profession, points>
  for (auto v = m_vecSkillData.begin(); v != m_vecSkillData.end(); ++v)
  {
    if (v->eProfession == EPROFESSION_NOVICE)
      continue;
    if(!checkInBranch(v->eProfession))
      continue;
    DWORD point = v->dwUsedPoint;
    auto it = find_if(vecPoints.begin(), vecPoints.end(), [&](const pair<EProfession, DWORD>& r) -> bool {
        return r.first == v->eProfession;
        });
    if (it != vecPoints.end())
      point += it->second;
    mapProPoint[v->eProfession] = point;
  }

  TVecDWORD rankPoints;
  for(auto& m : mapProPoint)
    rankPoints.push_back(m.second);

  DWORD size = rankPoints.size();
  if (size >= 2)
  {
    TVecDWORD vecAddPoints;
    for (auto &v : rankPoints) vecAddPoints.push_back(v);
    for (DWORD i = 0; i < size - 1; ++i)
    {
      vecAddPoints[i+1] += vecAddPoints[i];
    }

    for (DWORD i = 0; i < size - 1; ++i)
    {
      if (rankPoints[i+1] != 0)
      {
        if (vecAddPoints[i] < (i + 1) * ONE_JOB_POINT)
        {
          XERR << "[技能-升级], 前置技能不满足" << pUser->name << pUser->id << m_pFighter->getProfession() << "前置技能点数:" << vecAddPoints[i] << XEND;
          return false;
        }
      }
    }
  }

  // init data first
  TVecDWORD vecLearnSkill;
  for (auto v = vecSkills.begin(); v != vecSkills.end(); ++v)
  {
    SSkillItem* pItem = get<1>(*v);
    pItem->bActive = false;
    pItem->dwID = get<2>(*v);
    if (!pItem->bLearn)
    {
      vecLearnSkill.push_back(pItem->dwID);
      pItem->bLearn = true;

      if (pItem->pSkillCFG != nullptr && canEquip(pItem->pSkillCFG->getSkillType()) == true)
      {
        //setValidPos(pItem->dwID, pItem->dwSourceid);
        equipSkill(ESKILLSHORTCUT_NORMAL, pItem->dwID, pItem->dwSourceid);
        // 第一次技能槽溢出, 通知客户端提示玩家
        /*if (getPos(pItem->dwID, pItem->dwSourceid) == 0)
        {
          m_pFighter->getUser()->getUserSceneData().addFirstActionDone(EFIRSTACTION_SKILL_OVERFLOW);
        }
        */
      }
    }
  }

  // check skill can levelup
  for (auto v = vecSkills.begin(); v != vecSkills.end(); ++v)
  {
    SSkillItem* pItem = get<1>(*v);
    auto n = find(vecLearnSkill.begin(), vecLearnSkill.end(), pItem->dwID);
    if (n != vecLearnSkill.end())
    {
      if (pItem->pSkillCFG == nullptr || pItem->pSkillCFG->checkCondition(m_pFighter->getUser()) == false)
      {
        recover();
        return false;
      }
    }

    DWORD dwSkillID = pItem->pSkillCFG->getSkillID();
    const BaseSkill* pSkillCFG = pItem->pSkillCFG;
    while (dwSkillID != get<2>(*v))
    {
      dwSkillID = pSkillCFG->getNextSkillID();
      if(dwSkillID == 0)
      {
        bool blSystemForbide = MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_PEAK);
        bool blFunctionOpen = m_pFighter->getUser()->getMenu().isOpen(EMENUID_PEAK_LEVEL);
        if(blSystemForbide == false && blFunctionOpen == true)
          dwSkillID = pSkillCFG->getBreakSkillID();
        blSystemForbide = MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_PEAK);
        if (!blSystemForbide && dwSkillID == 0 && pUser->getUserSceneData().getDeadLv() >= pSkillCFG->getDeadLvReq())
          dwSkillID = pSkillCFG->getDeadSkillID();
      }
      pSkillCFG = SkillManager::getMe().getSkillCFG(dwSkillID);
      if (pSkillCFG == nullptr || pSkillCFG->checkCondition(m_pFighter->getUser()) == false)
      {
        recover();
        return false;
      }
    }
  }

  // dec point
  for (auto v = vecPoints.begin(); v != vecPoints.end(); ++v)
  {
    EProfession eProfession = v->first;
    auto o = find_if(m_vecSkillData.begin(), m_vecSkillData.end(), [eProfession](const SSkillData& r) -> bool{
      return r.eProfession == eProfession;
    });
    if (o != m_vecSkillData.end())
    {
      o->dwUsedPoint += v->second;
      o->dwPrimaryPoint += mapPrimaryPoint[eProfession];
    }
  }
  m_dwTotalPoint -= dwNeedPoint;
  m_pFighter->getUser()->setDataMark(EUSERDATATYPE_SKILL_POINT);

  // update final data
  for (auto v = vecSkills.begin(); v != vecSkills.end(); ++v)
  {
    SSkillItem& rOriItem = get<0>(*v);
    SSkillItem* pCurItem = get<1>(*v);

    pCurItem->bActive = false;
    pCurItem->bLearn = true;
    pCurItem->pSkillCFG = SkillManager::getMe().getSkillCFG(pCurItem->dwID);
    pCurItem->dwRuneSpecID = rOriItem.dwRuneSpecID;
    pCurItem->dwReplaceID = rOriItem.dwReplaceID;
    pCurItem->dwExtraLv = rOriItem.dwExtraLv;

    // update pos
    /*DWORD pos = getPos(rOriItem.dwID, rOriItem.dwSourceid);
    if (pos != 0)
    {
      setPos(rOriItem.dwID, rOriItem.dwSourceid, 0);
      setPos(pCurItem->dwID, pCurItem->dwSourceid, pos);
    }
    DWORD autopos = getAutoPos(rOriItem.dwID, rOriItem.dwSourceid);
    if (autopos != 0)
    {
      setAutoPos(rOriItem.dwID, rOriItem.dwSourceid, 0);
      setAutoPos(pCurItem->dwID, pCurItem->dwSourceid, autopos);
    }
    DWORD extendpos = getExtendPos(rOriItem.dwID, rOriItem.dwSourceid);
    if (extendpos != 0)
    {
      setExtendPos(rOriItem.dwID, rOriItem.dwSourceid, 0);
      setExtendPos(pCurItem->dwID, pCurItem->dwSourceid, extendpos);
    }*/
    for (auto &m : m_mapShortcut)
    {
      SPosData* pShortcut = getShortcut(m.first, rOriItem.dwID, rOriItem.dwSourceid);
      if (pShortcut != nullptr)
      {
        pShortcut->dwID = pCurItem->dwID;
        pShortcut->dwSourceid = pCurItem->dwSourceid;
      }
    }

    // passive skill to add buff
    m_pFighter->getUser()->m_oBuff.delSkillBuff(rOriItem.dwID);
    m_pFighter->getUser()->m_oBuff.addSkillBuff(pCurItem->dwID);
    addupdate(rOriItem.eProfession, rOriItem.dwID);
    addupdate(pCurItem->eProfession, pCurItem->dwID);
    DWORD dwSkillId = rOriItem.dwID / 1000;


    //plat log
    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eventType = EventType_Change_Skill;
    PlatLogManager::getMe().eventLog(thisServer,
      m_pFighter->getUser()->getUserSceneData().getPlatformId(),
      m_pFighter->getUser()->getZoneID(),
      m_pFighter->getUser()->accid,
      m_pFighter->getUser()->id,
      eid,
      m_pFighter->getUser()->getUserSceneData().getCharge(), eventType, 0, 1);

    PlatLogManager::getMe().changeLog(thisServer,
      m_pFighter->getUser()->getUserSceneData().getPlatformId(),
      m_pFighter->getUser()->getZoneID(),
      m_pFighter->getUser()->accid,
      m_pFighter->getUser()->id,
      eventType,
      eid,/*eid*/
      EChange_Skill, rOriItem.dwID, pCurItem->dwID, dwSkillId);
  }

  m_pFighter->getUser()->setCollectMark(ECOLLECTTYPE_DYNAMIC_BUFF);
  refreshEnableSkill();
  m_pFighter->getUser()->refreshDataAtonce();
  m_pFighter->getUser()->getEvent().onSkillPointChange();
  m_pFighter->getUser()->getMenu().refreshNewMenu(EMENUCOND_SKILL);
  m_pFighter->getUser()->updateEnsembleSkill();

  for (int i = 0; i < cmd.skillids_size(); ++i)
  {
    m_pFighter->getUser()->getEvent().onSkillLevelUp(cmd.skillids(i));
    XLOG << "[角色技能-升级]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "skillid:" << cmd.skillids(i) << XEND;
  }
  return true;
}

bool FighterSkill::equipSkill(const EquipSkill& cmd)
{
  if (m_pFighter == nullptr || m_pFighter->getUser() == nullptr)
    return false;

  SceneUser* pUser = m_pFighter->getUser();
  SPosData* pFrom = getShortcut(cmd.efrom(), cmd.skillid(), cmd.sourceid());
  SPosData* pTo = getShortcut(cmd.eto(), cmd.pos());

  if (pFrom == pTo)
  {
    XERR << "[角色技能-卡槽]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "从" << cmd.efrom() << "移动技能" << cmd.skillid() << "到" << cmd.eto() << cmd.pos() << "失败,快捷栏不能自身拖动" << XEND;
    return false;
  }

  if (hasSkill(cmd.skillid(), cmd.sourceid()) == false)
  {
    XERR << "[角色技能-卡槽]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "从" << cmd.efrom() << "移动技能" << cmd.skillid() << "到" << cmd.eto() << cmd.pos() << "失败,该技能非法" << XEND;
    return false;
  }

  // check pos valid
  if (pTo != nullptr)
  {
    if ((cmd.eto() == ESKILLSHORTCUT_NORMAL && cmd.pos() > m_dwMaxPos) || (cmd.eto() == ESKILLSHORTCUT_AUTO && isAutoPosValid(cmd.pos()) == false))
    {
      XERR << "[角色技能-卡槽]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
        << "从" << cmd.efrom() << "移动技能" << cmd.skillid() << "到" << cmd.eto() << cmd.pos() << "失败,该技能栏非法" << XEND;
      return false;
    }
    if (isExtend(cmd.eto()) == true)
    {
      DWORD dwNum = 0;
      for (size_t i = 0; i < VEC_SHORTCUT_EXTEND.size(); ++i)
      {
        if (VEC_SHORTCUT_EXTEND[i] == cmd.eto())
        {
          dwNum = i;
          break;
        }
      }
      if (dwNum * SKILL_MAX_CUTS + cmd.pos() > m_dwMaxExtendPos)
      {
        XERR << "[角色技能-卡槽]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
          << "从" << cmd.efrom() << "移动技能" << cmd.skillid() << "到" << cmd.eto() << cmd.pos() << "失败,该技能栏非法" << XEND;
        return false;
      }
    }
  }

  // clear old skillcut
  SPosData* pOri = getShortcut(cmd.eto(), cmd.skillid(), cmd.sourceid());
  if (pOri != nullptr && pOri != pFrom)
  {
    addPosUpdate(pOri->dwID, pOri->dwSourceid);
    pOri->clear();
  }

  // update shortcut
  if (pFrom == nullptr && pTo != nullptr)           // 新增
  {
    if (cmd.efrom() != ESKILLSHORTCUT_MIN)
    {
      XERR << "[角色技能-卡槽]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
        << "从" << cmd.efrom() << "移动技能" << cmd.skillid() << "到" << cmd.eto() << cmd.pos() << "失败,来源不是外部,但是未获取到快捷栏信息" << XEND;
      return false;
    }
    if (pTo->dwID != 0)
    {
      addPosUpdate(pTo->dwID, pTo->dwSourceid);
      pTo->clear();
    }

    pTo->dwID = cmd.skillid();
    pTo->dwSourceid = cmd.sourceid();
    addPosUpdate(pTo->dwID, pTo->dwSourceid);
  }
  else if (pFrom != nullptr && pTo == nullptr)    // 移除
  {
    if (cmd.eto() != ESKILLSHORTCUT_MIN)
    {
      XERR << "[角色技能-卡槽]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
        << "从" << cmd.efrom() << "移动技能" << cmd.skillid() << "到" << cmd.eto() << cmd.pos() << "失败,目标不是外部,但是未获取到快捷栏信息" << XEND;
      return false;
    }

    addPosUpdate(pFrom->dwID, pFrom->dwSourceid);
    pFrom->clear();
  }
  else if (pFrom != nullptr && pTo != nullptr)    // 交换
  {
    DWORD dwTemp = pFrom->dwID;
    pFrom->dwID = pTo->dwID;
    pTo->dwID = dwTemp;

    dwTemp = pFrom->dwSourceid;
    pFrom->dwSourceid = pTo->dwSourceid;
    pTo->dwSourceid = dwTemp;

    addPosUpdate(pFrom->dwID, pFrom->dwSourceid);
    addPosUpdate(pTo->dwID, pTo->dwSourceid);
  }
  else
  {
    XERR << "[角色技能-卡槽]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "从" << cmd.efrom() << "移动技能" << cmd.skillid() << "到" << cmd.eto() << cmd.pos() << "失败,未知错误" << XEND;
    return false;
  }

  update(now());
  XLOG << "[角色技能-卡槽]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "从" << cmd.efrom() << "移动技能" << cmd.skillid() << cmd.sourceid() << "到" << cmd.eto() << cmd.pos() << "成功" << XEND;
  return true;
}

/*bool FighterSkill::equipSkill(ESkillShortcut eType, DWORD skillid, DWORD sourceid, DWORD pos)
{
  if ((bauto && !isRightAutoPos(pos)) || (!bauto && pos > m_dwMaxPos))
    return false;
  if (isSkillEnable(skillid) == false)
    return false;

  if (getPosSkill(pos, bauto) != nullptr)
    return false;
  if (bauto)
    setAutoPos(skillid, 0, pos);
  else
    setPos(skillid, 0, pos);

  addPosUpdate(skillid, 0);
  return true;
}*/

bool FighterSkill::equipSkill(ESkillShortcut eType, DWORD skillid, DWORD sourceid)
{
  if (m_pFighter == nullptr || m_pFighter->getUser() == nullptr)
    return false;

  SceneUser* pUser = m_pFighter->getUser();
  if (hasSkill(skillid, sourceid) == false)
  {
    XERR << "[角色技能-卡槽]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "在 type :" << eType << "自动装备技能 :" << skillid << sourceid << "失败,该技能不可用" << XEND;
    return false;
  }

  SPosData* pData = getShortcut(eType, skillid, sourceid);
  if (pData != nullptr)
    return false;

  auto m = m_mapShortcut.find(eType);
  if (m == m_mapShortcut.end())
    return false;

  TVecPosSkill& vecPos = m->second;
  for (auto &v : vecPos)
  {
    if (v.dwPos == 0)
      continue;
    if ((eType == ESKILLSHORTCUT_NORMAL && v.dwPos > m_dwMaxPos) || (eType == ESKILLSHORTCUT_AUTO && isAutoPosValid(v.dwPos) == false))
      continue;
    if (v.dwID == 0)
    {
      v.dwID = skillid;
      v.dwSourceid = sourceid;
      addPosUpdate(v.dwID, v.dwSourceid);
      XLOG << "[角色技能-卡槽]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "在 type :" << eType << "自动装备技能 :" << skillid << sourceid << "成功,装备在pos :" << v.dwPos << "上" << XEND;
      break;
    }
  }

  return true;
}

bool FighterSkill::equipSkill(ESkillShortcut eType, DWORD dwSkillID, DWORD dwSourceID, DWORD dwPos)
{
  if (m_pFighter == nullptr || m_pFighter->getUser() == nullptr)
    return false;

  SceneUser* pUser = m_pFighter->getUser();
  if (hasSkill(dwSkillID, dwSourceID) == false)
  {
    XERR << "[角色技能-卡槽]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "在type :" << eType << "设置技能 :" << dwSkillID << dwSourceID << "到pos :" << dwPos << "失败,技能不可用" << XEND;
    return false;
  }
  if ((eType == ESKILLSHORTCUT_NORMAL && dwPos > m_dwMaxPos) || (eType == ESKILLSHORTCUT_AUTO && isAutoPosValid(dwPos) == false))
  {
    XERR << "[角色技能-卡槽]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "在type :" << eType << "设置技能 :" << dwSkillID << dwSourceID << "到pos :" << dwPos << "失败,pos不可用" << XEND;
    return false;
  }

  SPosData* pDest = getShortcut(eType, dwPos);
  if (pDest == nullptr)
    return false;

  SPosData* pData = getShortcut(eType, dwSkillID, dwSourceID);
  if (pData != nullptr)
  {
    addPosUpdate(pData->dwID, pData->dwSourceid);
    pData->clear();
  }

  if (pDest->dwID == dwSkillID && pDest->dwSourceid == dwSourceID)
    return false;

  if (pDest->dwID != 0)
  {
    addPosUpdate(pDest->dwID, pDest->dwSourceid);
    pDest->clear();
  }

  pDest->dwID = dwSkillID;
  pDest->dwSourceid = dwSourceID;
  addPosUpdate(dwSkillID, dwSourceID);
  XLOG << "[角色技能-卡槽]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "在type :" << eType << "设置技能 :" << dwSkillID << dwSourceID << "到pos :" << dwPos << "成功" << XEND;
  return true;
}

/*bool FighterSkill::equipSkill(const EquipSkill& clientCmd)
{
  if (m_pFighter == nullptr || m_pFighter->getUser() == nullptr)
    return false;

  // get info from cmd
  DWORD pos = clientCmd.pos();
  DWORD skillid = clientCmd.skillid();
  DWORD sourceid = clientCmd.sourceid();
  bool isFromExtend = clientCmd.efrom() == ESKILLSHORTCUT_EXTEND;
  bool isFromNormal = clientCmd.efrom() == ESKILLSHORTCUT_NORMAL;

  bool fromNormal = (isFromNormal || isFromExtend);
  bool fromPanel = clientCmd.efrom() == ESKILLSHORTCUT_MIN;
  bool fromAuto = clientCmd.efrom() == ESKILLSHORTCUT_AUTO;

  bool isToExtend = clientCmd.eto() == ESKILLSHORTCUT_EXTEND;
  bool isToNormal = clientCmd.eto() == ESKILLSHORTCUT_NORMAL;

  bool toNormal = (isToNormal || isToExtend);
  bool toAuto = clientCmd.eto() == ESKILLSHORTCUT_AUTO;
  bool toPanel = clientCmd.eto() == ESKILLSHORTCUT_MIN;

  if ((isFromExtend && isToNormal) || (isFromNormal && isToExtend))
    return false;
  bool isAboutExtend = (isFromExtend || isToExtend);

  if (fromPanel && toPanel)
    return false;

  // check illegal pos
  if (isToNormal && pos > m_dwMaxPos)
    return false;
  if (isToExtend && pos > m_dwMaxExtendPos)
    return false;
  if (toAuto && !isAutoPosValid(pos))
    return false;

  auto getNormalPos = [&](DWORD skillid, DWORD sourceid) -> DWORD
  {
    return isAboutExtend ? getExtendPos(skillid, sourceid) : getPos(skillid, sourceid);
  };
  auto setNormalPos = [&](DWORD skillid, DWORD sourceid, DWORD pos)
  {
    if (isAboutExtend)
      setExtendPos(skillid, sourceid, pos);
    else
      setPos(skillid, sourceid, pos);
  };

  // get "from skill" info
  DWORD frompos = 0;
  if (!fromPanel)
    frompos = fromNormal ? getNormalPos(skillid, sourceid) : getAutoPos(skillid, sourceid);
  else
    frompos = toNormal ? getNormalPos(skillid, sourceid) : getAutoPos(skillid, sourceid);

  // get "to skill" info
  SPosData* pToPosSkill = nullptr;
  DWORD toid = 0;
  DWORD tosourceid = 0;
  if (toNormal || toAuto)
    pToPosSkill = getPosSkill(pos, toAuto, isAboutExtend);
  if (pToPosSkill != nullptr)
  {
    toid = pToPosSkill->dwID;
    tosourceid = pToPosSkill->dwSourceid;
  }

  // only about normal slot
  if (!fromAuto && !toAuto)
  {
    if (pToPosSkill != nullptr)
    {
      setNormalPos(toid, tosourceid, frompos);
      setNormalPos(skillid, sourceid, pos);
      XLOG << "[技能-卡槽], 玩家:" << m_pFighter->getUser()->name << m_pFighter->getUser()->id << "普通技能槽位置变化, 技能A:" << skillid << sourceid << frompos << "替换技能B:" << toid << tosourceid << pos << XEND;
    }
    else
    {
      setNormalPos(skillid, sourceid, pos);
      XLOG << "[技能-卡槽], 玩家:" << m_pFighter->getUser()->name << m_pFighter->getUser()->id << "普通技能槽位置变化, 技能A:" << skillid << sourceid << "拖到到位置:" << pos << XEND;
    }
  }
  // only about auto slot
  else if (!fromNormal && !toNormal)
  {
    if (pToPosSkill != nullptr)
    {
      setAutoPos(toid, tosourceid, frompos);
      setAutoPos(skillid, sourceid, pos);
      XLOG << "[技能-卡槽], 玩家:" << m_pFighter->getUser()->name << m_pFighter->getUser()->id << "自动技能槽位置变化, 技能A:" << skillid << sourceid << frompos << "替换技能B:" << toid << tosourceid << pos << XEND;
    }
    else
    {
      setAutoPos(skillid, sourceid, pos);
      XLOG << "[技能-卡槽], 玩家:" << m_pFighter->getUser()->name << m_pFighter->getUser()->id << "自动技能槽位置变化, 技能A:" << skillid << sourceid << "拖到到位置:" << pos << XEND;
    }
  }
  // from normal & to auto
  else if (fromNormal && toAuto)
  {
    if (pToPosSkill != nullptr)
    {
      setAutoPos(skillid, sourceid, pos);
      setNormalPos(toid, tosourceid, frompos);
      XLOG << "[技能-卡槽], 玩家:" << m_pFighter->getUser()->name << m_pFighter->getUser()->id << "技能槽位置变化, 普通槽技能A:" << skillid << sourceid << frompos << "替换自动技能B:" << toid << tosourceid << pos << XEND;
    }
    else
    {
      setAutoPos(skillid, sourceid, pos);
      setNormalPos(skillid, sourceid, 0);
      XLOG << "[技能-卡槽], 玩家:" << m_pFighter->getUser()->name << m_pFighter->getUser()->id << "普通技能槽技能A:" << skillid << sourceid << frompos << "拖动到自动技能槽位置:" << pos << XEND;
    }
  }
  // from auto & to normal
  else if (fromAuto && toNormal)
  {
    if (pToPosSkill != nullptr)
    {
      setNormalPos(skillid, sourceid, pos);
      setAutoPos(toid, tosourceid, frompos);
      XLOG << "[技能-卡槽], 玩家:" << m_pFighter->getUser()->name << m_pFighter->getUser()->id << "技能槽位置变化, 自动槽技能A:" << skillid << sourceid << frompos << "替换普通槽技能B:" << toid << tosourceid << pos << XEND;
    }
    else
    {
      setNormalPos(skillid, sourceid, pos);
      setAutoPos(skillid, sourceid, 0);
      XLOG << "[技能-卡槽], 玩家:" << m_pFighter->getUser()->name << m_pFighter->getUser()->id << "自动技能槽技能A:" << skillid << sourceid << frompos << "拖动到普通技能槽位置:" << pos << XEND;
    }
  }

  // add update
  addPosUpdate(skillid, sourceid);
  if (pToPosSkill != nullptr)
    addPosUpdate(toid, tosourceid);

  return true;
}*/

/*
bool FighterSkill::equipSkill(const EquipSkill& clientCmd)
{
  if (m_pFighter == nullptr || m_pFighter->getUser() == nullptr)
    return false;
  SceneUser* pUser = m_pFighter->getUser();

  // get info from cmd
  DWORD pos = clientCmd.pos();
  DWORD skillid = clientCmd.skillid();
  DWORD sourceid = clientCmd.sourceid();
  bool fromPanel = clientCmd.efrom() == ESKILLSHORTCUT_MIN;
  bool fromNormal = clientCmd.efrom() == ESKILLSHORTCUT_NORMAL;
  bool fromAuto = clientCmd.efrom() == ESKILLSHORTCUT_AUTO;
  bool toNormal = clientCmd.eto() == ESKILLSHORTCUT_NORMAL;
  bool toAuto = clientCmd.eto() == ESKILLSHORTCUT_AUTO;

  // check illegal pos
  if (toNormal && pos > m_dwMaxPos)
    return false;
  if (toAuto && pos > m_dwAutoMaxPos)
    return false;

  // get skill object
  SceneFighter* pNoviceFighter = pUser->getFighter(EPROFESSION_NOVICE);
  SSkillItem* pItem = nullptr;
  for (auto v = m_vecSkillData.begin(); v != m_vecSkillData.end(); ++v)
  {
    pItem = v->getSkillItem(skillid, sourceid);
    if (pItem != nullptr)
      break;
  }
  if (pItem == nullptr)
  {
    if (pNoviceFighter != nullptr)
    {
      SSkillData* pData = pNoviceFighter->getSkill().getSkillData(EPROFESSION_NOVICE);
      if (pData != nullptr)
        pItem = pData->getSkillItem(skillid, sourceid);
    }
  }
  if (pItem == nullptr)
    return false;

  // check can equip
  if (pItem->pSkillCFG == nullptr)
    return false;
  if (canEquip(pItem->pSkillCFG->getSkillType()) == false)
    return false;
  if (toAuto && pItem->pSkillCFG->canEquipAuto() == false)
    return false;
  if (toAuto && pItem->dwAutoPos == pos)
    return false;
  if (toNormal && pItem->dwPos == pos)
    return false;

  // 脱掉技能
  if (pos == 0 && !fromPanel)
  {
    if (fromAuto)
    {
      DWORD autonum = 0;
      for (auto &v : m_vecSkillData)
      {
        for (auto &s : v.vecSkillItem)
          autonum = s.dwAutoPos != 0 ? autonum + 1 : autonum;
      }
      SSkillData* pData = pNoviceFighter->getSkill().getSkillData(EPROFESSION_NOVICE);
      if (pData != nullptr)
      {
        for (auto &s : pData->vecSkillItem)
          autonum = s.dwAutoPos != 0 ? autonum + 1 : autonum;
      }
      // 自动槽至少保留一个技能
      if (autonum <= 1)
        return false;
    }

    fromNormal ? pItem->dwPos = 0 : pItem->dwAutoPos = 0;
    addupdate(pItem->eProfession, pItem->dwID, pItem->dwSourceid);
    m_pFighter->getUser()->setDataMark(EUSERDATATYPE_FIGHTER);
    return true;
  }

  // get target pos skill object
  SSkillItem* pOldItem = nullptr;
  for (auto v = m_vecSkillData.begin(); v != m_vecSkillData.end(); ++v)
  {
    pOldItem = v->getSkillItemByPos(pos, toAuto);
    if (pOldItem != nullptr)
      break;
  }
  if (pOldItem == nullptr)
  {
    SSkillData* pData = pNoviceFighter->getSkill().getSkillData(EPROFESSION_NOVICE);
    if (pData != nullptr)
      pOldItem = pData->getSkillItemByPos(pos, toAuto);
  }


  // exchange postion (not normalslot and autoslot)
  if (fromPanel || (fromNormal && !toAuto) || (fromAuto && !toNormal))
  {
    bool handleNormalSlot = fromNormal || toNormal;
    // skill/shadow -> skill/shadow
    if (pOldItem != nullptr)
    {
      DWORD oldpos = handleNormalSlot ? pItem->dwPos : pItem->dwAutoPos;

      if (handleNormalSlot)
      {
        pItem->dwPos = pOldItem->dwPos;
        pOldItem->dwPos = oldpos;
      }
      else
      {
        pItem->dwAutoPos = pOldItem->dwAutoPos;
        pOldItem->dwAutoPos = oldpos;
      }

      addupdate(pItem->eProfession, pItem->dwID, pItem->dwSourceid);
      addupdate(pOldItem->eProfession, pOldItem->dwID, pOldItem->dwSourceid);
    }
    // skill/shadow -> blank, Equip off
    else
    {
      if (handleNormalSlot)
        pItem->dwPos = pos;
      else
        pItem->dwAutoPos = pos;
      addupdate(pItem->eProfession, pItem->dwID, pItem->dwSourceid);
    }
  }
  // exchange between normal slot and auto slot
  else if ((fromNormal && toAuto) || (fromAuto && toNormal))
  {
    if (pOldItem != nullptr)
    {
      DWORD oldpos = fromNormal ? pItem->dwPos : pItem->dwAutoPos;
      if (fromNormal)
      {
        pItem->dwAutoPos = pOldItem->dwAutoPos; 
        pItem->dwPos = 0;

        pOldItem->dwPos = oldpos;
        pOldItem->dwAutoPos = 0;
      }
      else
      {
        pItem->dwPos = pOldItem->dwPos;
        pItem->dwAutoPos = 0;

        pOldItem->dwAutoPos = oldpos;
        pOldItem->dwPos = 0;
      }

      addupdate(pItem->eProfession, pItem->dwID, pItem->dwSourceid);
      addupdate(pOldItem->eProfession, pOldItem->dwID, pOldItem->dwSourceid);
    }
    else
    {
      if (fromNormal)
      {
        pItem->dwPos = 0;
        pItem->dwAutoPos = pos;
      }
      else
      {
        pItem->dwAutoPos = 0;
        pItem->dwPos = pos;
      }
      addupdate(pItem->eProfession, pItem->dwID, pItem->dwSourceid);
    }
  }

  pUser->setDataMark(EUSERDATATYPE_FIGHTER);
  XLOG("[技能装备] %llu, %llu, %u, %s, skillid : %u, pos : %u, source : %u", pUser->accid, pUser->id, pUser->getProfession(), pUser->name, skillid, pos, sourceid);
  return true;
}
*/

// 技能消耗时调用
void FighterSkill::refresheConsume()
{
  if (m_pFighter == nullptr)
    return;
  SSkillData* pData = getSkillData(EPROFESSION_NOVICE);
  if (pData == nullptr)
    return;
  DWORD maxvalue = MiscConfig::getMe().getSPurifyCFG().dwMaxPurify;
  for (auto v = pData->vecSkillItem.begin(); v != pData->vecSkillItem.end(); ++v)
  {
    if ((*v).pSkillCFG == nullptr || !(*v).pSkillCFG->isConsumeSkill())
      continue;
    addupdate(EPROFESSION_NOVICE, (*v).dwID);
    if (m_pFighter->getUser()->getPurify() < maxvalue)
      (*v).consume.reset();
  }
}

// timer调用
void FighterSkill::updateConsume(DWORD cur)
{
  if (m_pFighter == nullptr || m_pFighter->getUser() == nullptr)
    return;
  SSkillData* pData = getSkillData(EPROFESSION_NOVICE);
  if (pData == nullptr)
    return;

  DWORD maxvalue = MiscConfig::getMe().getSPurifyCFG().dwMaxPurify;
  DWORD interval = MiscConfig::getMe().getSPurifyCFG().dwGainInterval;
  if (interval == 0)
  {
    SceneUser* pUser = m_pFighter->getUser();
    XERR << "[角色技能]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "purify 恢复间隔为0, FighterSkill::updateConsume" << XEND;
    return;
  }
  for (auto v = pData->vecSkillItem.begin(); v != pData->vecSkillItem.end(); ++v)
  {
    if ((*v).pSkillCFG == nullptr || !(*v).pSkillCFG->isConsumeSkill())
      continue;
    SSkillConsume& consume = (*v).consume;
    if (cur < consume.nexttime)
      continue;
    DWORD nowvalue = m_pFighter->getUser()->getPurify();
    if (nowvalue >= maxvalue)
      continue;
    DWORD gain = (cur - consume.nexttime) / interval + 1; // not direct add 1, for load from sql
    m_pFighter->getUser()->addPurify(gain);
    consume.nexttime = (interval - (cur - consume.nexttime) % interval) + cur;
    addupdate(EPROFESSION_NOVICE, (*v).dwID);
  }
}

void FighterSkill::update(DWORD cur)
{
  if (m_pFighter == nullptr || m_pFighter->getUser() == nullptr)
    return;
  if (m_pFighter->getUser()->getInitChar() == false)
    return;

  updateConsume(cur);
  updateSpecSkill();
  if (m_vecUpdateIDs.empty() == true)
    return;

  SceneFighter* pNoviceFighter = m_pFighter->getUser()->getFighter(EPROFESSION_NOVICE);

  set<pair<DWORD, DWORD>> indexToDel;
  SkillUpdate cmd;
  for (auto v = m_vecUpdateIDs.begin(); v != m_vecUpdateIDs.end(); ++v)
  {
    SSkillData* pData = nullptr;
    if (v->eProfession == EPROFESSION_NOVICE && pNoviceFighter != nullptr)
      pData = pNoviceFighter->getSkill().getSkillData(EPROFESSION_NOVICE);
    else
      pData = getSkillData(v->eProfession);
    if (pData == nullptr)
      continue;

    SkillData* update = cmd.add_update();
    SkillData* del = cmd.add_del();
    if (update == nullptr || del == nullptr)
      continue;

    update->set_profession(pData->eProfession);
    update->set_usedpoint(pData->dwUsedPoint);
    update->set_primarypoint(pData->dwPrimaryPoint);
    del->set_profession(pData->eProfession);

    for (auto o = v->vecSkillItem.begin(); o != v->vecSkillItem.end(); ++o)
    {
      SSkillItem* pItem = pData->getSkillItem(o->dwID, o->dwSourceid);
      if (pItem != nullptr)
      {
        //if (pItem->eProfession == EPROFESSION_NOVICE && !pItem->bActive)
          //continue;
        if (pItem->bShadow && isInSlot(pItem->dwID, pItem->dwSourceid) == false)
        {
          indexToDel.insert(pair<DWORD, DWORD>(pItem->dwID, pItem->dwSourceid));
        }

        SkillItem* item = update->add_items();
        if (item != nullptr)
          pItem->toData(item, m_pFighter);
      }
      else
      {
        SkillItem* item = del->add_items();
        if (item != nullptr)
        {
          item->set_id(o->dwID);
          item->set_sourceid(o->dwSourceid);
        }
      }
    }
  }
  SSkillData* pData = nullptr;
  if (pNoviceFighter != nullptr)
      pData = pNoviceFighter->getSkill().getSkillData(EPROFESSION_NOVICE);
  if (pData != nullptr)
  {
    for (auto &s : indexToDel)
    {
      auto v = find_if (pData->vecSkillItem.begin(), pData->vecSkillItem.end(), [&s](const SSkillItem& r) ->bool {
          return r.dwID == s.first && r.dwSourceid == s.second;
          });
      if (v != pData->vecSkillItem.end())
        pData->vecSkillItem.erase(v);
    }
  }

  if (cmd.update_size() > 0 || cmd.del_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pFighter->getUser()->sendCmdToMe(send, len);

#ifdef _DEBUG
    SceneUser* pUser = m_pFighter->getUser();
    for (int i = 0; i < cmd.update_size(); ++i)
      XDBG << "[角色技能-更新]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "update:" << cmd.update(i).ShortDebugString() << XEND;
    for (int i = 0; i < cmd.del_size(); ++i)
      XDBG << "[角色技能-删除]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "del:" << cmd.del(i).ShortDebugString() << XEND;
#endif
  }

  m_vecUpdateIDs.clear();
}

SSkillData* FighterSkill::getSkillData(EProfession eProfession)
{
  auto v = find_if(m_vecSkillData.begin(), m_vecSkillData.end(), [eProfession](const SSkillData& r) -> bool{
    return r.eProfession == eProfession;
  });
  if (v == m_vecSkillData.end())
  {
    SSkillData stData;
    stData.eProfession = eProfession;
    m_vecSkillData.push_back(stData);
    v = find_if(m_vecSkillData.begin(), m_vecSkillData.end(), [eProfession](const SSkillData& r) -> bool{
      return r.eProfession == eProfession;
    });
  }
  if (v != m_vecSkillData.end())
    return &(*v);

  return nullptr;
}

void FighterSkill::addupdate(EProfession profession, DWORD id, DWORD sourceid)
{
  auto v = find_if(m_vecUpdateIDs.begin(), m_vecUpdateIDs.end(), [profession](const SSkillData& r) -> bool{
    return r.eProfession == profession;
  });
  if (v == m_vecUpdateIDs.end())
  {
    SSkillData stData;
    stData.eProfession = profession;
    SSkillItem stItem;
    stItem.dwID = id;
    stItem.dwSourceid = sourceid;
    stData.vecSkillItem.push_back(stItem);
    m_vecUpdateIDs.push_back(stData);
    return;
  }

  auto o = find_if(v->vecSkillItem.begin(), v->vecSkillItem.end(), [id, sourceid](const SSkillItem& r) -> bool{
    return r.dwID == id && r.dwSourceid == sourceid;
  });
  if (o == v->vecSkillItem.end())
  {
    SSkillItem stItem;
    stItem.dwID = id;
    stItem.dwSourceid = sourceid;
    v->vecSkillItem.push_back(stItem);
  }
}

bool FighterSkill::resetSkill()
{
  if (m_pFighter == nullptr || m_pFighter->getUser() == nullptr)
    return false;

  if(m_pFighter->getUser()->m_oBooth.hasOpen())
    return false;

  for (auto v = m_vecSkillData.begin(); v != m_vecSkillData.end(); ++v)
  {
    if (v->eProfession == EPROFESSION_NOVICE)
      continue;

    for (auto o = v->vecSkillItem.begin(); o != v->vecSkillItem.end(); ++o)
    {
      m_pFighter->getUser()->m_oBuff.delSkillBuff(o->dwID);
      m_pFighter->getUser()->getEvent().onDelSkill(o->dwID);

      removeShortcut(ESKILLSHORTCUT_MIN, o->dwID, o->dwSourceid);
      /*setPos(o->dwID, o->dwSourceid, 0);
      setAutoPos(o->dwID, o->dwSourceid, 0);
      setExtendPos(o->dwID, o->dwSourceid, 0);*/
      addupdate(v->eProfession, o->dwID);
    }

    m_dwTotalPoint += v->dwUsedPoint;
    v->dwUsedPoint = 0;
    v->vecSkillItem.clear();
    v->dwPrimaryPoint = 0;
  }

  refreshEnableSkill();
  if (m_pFighter->getUser() != nullptr)
  {
    m_pFighter->getUser()->setDataMark(EUSERDATATYPE_SKILL_POINT);
    m_pFighter->getUser()->refreshDataAtonce();
    m_pFighter->getUser()->getPackage().checkInvalidEquip(true);
    m_pFighter->getUser()->getMenu().checkBackoffSkillMenu();
    m_pFighter->getUser()->getTip().onSkillPoint();
  }

  m_pFighter->getUser()->getEvent().onSkillReset();
  m_pFighter->getUser()->updateEnsembleSkill();

  SceneUser* pUser = m_pFighter->getUser();
  if (pUser)
  {
    //plat log
    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eventType = EventType_Change_ResetSkillPoint;
    PlatLogManager::getMe().eventLog(thisServer,
      pUser->getUserSceneData().getPlatformId(),
      pUser->getZoneID(),
      pUser->accid,
      pUser->id,
      eid,
      pUser->getUserSceneData().getCharge(), eventType, 0, 1);

    PlatLogManager::getMe().changeLog(thisServer,
      pUser->getUserSceneData().getPlatformId(),
      pUser->getZoneID(),
      pUser->accid,
      pUser->id,
      eventType,
      eid,/*eid*/
      EChange_ResetSkillPoint, 0, 0, 0);

    XLOG << "[角色技能-重置]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "totalpoint:" << m_dwTotalPoint << XEND;

    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_RESET_POINT_SUM, pUser->getProfession(), SKILL_POINT_REST,0, (DWORD)1);
    if (pUser->m_oUserStat.checkAndSet(ESTATTYPE_RESET_POINT, SKILL_POINT_REST))
      pUser->m_oUserStat.sendStatLog(ESTATTYPE_RESET_POINT, pUser->getProfession(), SKILL_POINT_REST, 0, (DWORD)1);
  }
  return true;
}

void FighterSkill::getCurSkills(TVecDWORD& vecIDs)
{
  if (!m_pFighter || !m_pFighter->getUser())
    return;
  SceneFighter* pNoviceFighter = m_pFighter->getUser()->getFighter(EPROFESSION_NOVICE);
  if (pNoviceFighter != nullptr && pNoviceFighter != m_pFighter)
    pNoviceFighter->getSkill().getCurSkills(vecIDs);

  for (auto m = m_vecSkillData.begin(); m != m_vecSkillData.end(); ++m)
  {
    for (auto v = (*m).vecSkillItem.begin(); v != (*m).vecSkillItem.end(); ++v)
    {
      if ((*v).bLearn == false)
        continue;
      vecIDs.push_back((*v).dwID);
    }
  }
}

DWORD FighterSkill::getSkillLv(DWORD skillid)
{
  for (auto m = m_vecSkillData.begin(); m != m_vecSkillData.end(); ++m)
  {
    for (auto v = (*m).vecSkillItem.begin(); v != (*m).vecSkillItem.end(); ++v)
    {
      if ((*v).bLearn == false || (*v).bShadow)
        continue;
      if (skillid / 1000 == (*v).dwID / 1000)
        return (*v).dwID - (*v).dwID / 1000 * 1000;
    }
  }
  if (m_pFighter->getProfession() != EPROFESSION_NOVICE)
  {
    SceneFighter* pNoviceFighter = m_pFighter->getUser()->getFighter(EPROFESSION_NOVICE);
    if (pNoviceFighter != nullptr)
      return pNoviceFighter->getSkill().getSkillLv(skillid);
  }
  return 0;
}

/*void FighterSkill::setPos(DWORD skillid, DWORD sourceid, DWORD pos)
{
  if (pos > m_dwMaxPos)
    return;

  auto it = find_if(m_vecPosSkill.begin(), m_vecPosSkill.end(), [&](const SPosData& r) -> bool{
      return r.dwID == skillid && r.dwSourceid == sourceid;
      });
  if (it != m_vecPosSkill.end())
    m_vecPosSkill.erase(it);

  if (pos == 0)
    return;

  it = find_if(m_vecPosSkill.begin(), m_vecPosSkill.end(), [&](const SPosData& r) -> bool{
      return r.dwPos == pos;
      });
  if (it != m_vecPosSkill.end())
    m_vecPosSkill.erase(it);

  SPosData sData;
  sData.dwID = skillid;
  sData.dwSourceid = sourceid;
  sData.dwPos = pos;
  m_vecPosSkill.push_back(sData);
}

void FighterSkill::setAutoPos(DWORD skillid, DWORD sourceid, DWORD pos)
{
  if (!isRightAutoPos(pos))
    return;

  auto it = find_if(m_vecAutoPosSkill.begin(), m_vecAutoPosSkill.end(), [&](const SPosData& r) -> bool{
      return r.dwID == skillid && r.dwSourceid == sourceid;
      });
  if (it != m_vecAutoPosSkill.end())
    m_vecAutoPosSkill.erase(it);

  if (pos == 0)
    return;

  it = find_if(m_vecAutoPosSkill.begin(), m_vecAutoPosSkill.end(), [&](const SPosData& r) -> bool{
      return r.dwPos == pos;
      });
  if (it != m_vecAutoPosSkill.end())
    m_vecAutoPosSkill.erase(it);

  SPosData sData;
  sData.dwID = skillid;
  sData.dwSourceid = sourceid;
  sData.dwPos = pos;
  m_vecAutoPosSkill.push_back(sData);
}

void FighterSkill::setExtendPos(DWORD skillid, DWORD sourceid, DWORD pos)
{
  if (pos > m_dwMaxExtendPos)
    return;

  auto it = find_if(m_vecExtendPosSkill.begin(), m_vecExtendPosSkill.end(), [&](const SPosData& r) ->bool {
      return r.dwID == skillid && r.dwSourceid == sourceid;
      });
  if (it != m_vecExtendPosSkill.end())
    m_vecExtendPosSkill.erase(it);
  if (pos == 0)
    return;

  it = find_if(m_vecExtendPosSkill.begin(), m_vecExtendPosSkill.end(), [&](const SPosData& r) -> bool {
      return r.dwPos == pos;
      });
  if (it != m_vecExtendPosSkill.end())
    m_vecExtendPosSkill.erase(it);

  SPosData sData;
  sData.dwID = skillid;
  sData.dwSourceid = sourceid;
  sData.dwPos = pos;
  m_vecExtendPosSkill.push_back(sData);
}*/

/*void FighterSkill::setValidPos(DWORD skillid, DWORD sourceid)
{
  auto v = find_if(m_vecPosSkill.begin(), m_vecPosSkill.end(), [&](const SPosData& r) ->bool{
      return r.dwID == skillid && r.dwSourceid == sourceid;
      });
  if (v != m_vecPosSkill.end())
    return;

  for (DWORD i = 1; i <= m_dwMaxPos; ++i)
  {
    auto v = find_if(m_vecPosSkill.begin(), m_vecPosSkill.end(), [&](const SPosData& r) ->bool{
        return r.dwPos == i;
        });
    if (v == m_vecPosSkill.end())
    {
      SPosData sData;
      sData.dwID = skillid;
      sData.dwSourceid = sourceid;
      sData.dwPos = i;
      m_vecPosSkill.push_back(sData);
      return;
    }
  }
}*/

/*void FighterSkill::setValidAutoPos(DWORD skillid, DWORD sourceid)
{
  auto v = find_if(m_vecAutoPosSkill.begin(), m_vecAutoPosSkill.end(), [&](const SPosData& r) ->bool{
      return r.dwID == skillid && r.dwSourceid == sourceid;
      });
  if (v != m_vecAutoPosSkill.end())
    return;

  for (DWORD i = 1; i <= m_dwAutoMaxPos; ++i)
  {
    auto v = find_if(m_vecAutoPosSkill.begin(), m_vecAutoPosSkill.end(), [&](const SPosData& r) ->bool{
        return r.dwPos == i;
        });
    if (v == m_vecAutoPosSkill.end())
    {
      SPosData sData;
      sData.dwID = skillid;
      sData.dwSourceid = sourceid;
      sData.dwPos = i;
      m_vecAutoPosSkill.push_back(sData);
      return;
    }
  }
  
  //monthcard skill pos
  auto v1 = find_if(m_vecAutoPosSkill.begin(), m_vecAutoPosSkill.end(), [&](const SPosData& r) ->bool {
    return r.dwPos == MONTH_CARD_SKILL_POS;
  });
  if (v1 == m_vecAutoPosSkill.end())
  {
    SPosData sData;
    sData.dwID = skillid;
    sData.dwSourceid = sourceid;
    sData.dwPos = MONTH_CARD_SKILL_POS;
    m_vecAutoPosSkill.push_back(sData);
    return;
  }

}*/

/*DWORD FighterSkill::getPos(DWORD skillid, DWORD sourceid)
{
  auto v = find_if(m_vecPosSkill.begin(), m_vecPosSkill.end(), [&](const SPosData& r) ->bool{
      return r.dwID == skillid && r.dwSourceid == sourceid;
      });
  if (v != m_vecPosSkill.end())
    return v->dwPos;
  return 0;
}

DWORD FighterSkill::getAutoPos(DWORD skillid, DWORD sourceid)
{
  auto v = find_if(m_vecAutoPosSkill.begin(), m_vecAutoPosSkill.end(), [&](const SPosData& r) ->bool{
      return r.dwID == skillid && r.dwSourceid == sourceid;
      });
  if (v != m_vecAutoPosSkill.end())
    return v->dwPos;
  return 0;
}

DWORD FighterSkill::getExtendPos(DWORD skillid, DWORD sourceid)
{
  auto v = find_if(m_vecExtendPosSkill.begin(), m_vecExtendPosSkill.end(), [&](const SPosData& r) ->bool {
      return r.dwID == skillid && r.dwSourceid == sourceid;
      });
  if (v != m_vecExtendPosSkill.end())
    return v->dwPos;
  return 0;
}*/

SPosData* FighterSkill::getPosSkill(DWORD pos, bool autoOrNormal, bool bExtend /*=false*/)
{
  if (autoOrNormal)
  {
    SPosData* pData = getShortcut(ESKILLSHORTCUT_AUTO, pos);
    if (pData != nullptr && pData->dwID != 0)
      return pData;
    return nullptr;
    /*auto v = find_if(m_vecAutoPosSkill.begin(), m_vecAutoPosSkill.end(), [&](const SPosData& r) ->bool{
        return r.dwPos == pos;
        });
    if (v == m_vecAutoPosSkill.end())
      return nullptr;
    return &(*v);*/
  }
  else
  {
    if (!bExtend)
    {
      SPosData* pData = getShortcut(ESKILLSHORTCUT_NORMAL, pos);
      if (pData != nullptr && pData->dwID != 0)
        return pData;
      return nullptr;
      /*auto v = find_if(m_vecPosSkill.begin(), m_vecPosSkill.end(), [&](const SPosData& r) ->bool{
          return r.dwPos == pos;
          });
      if (v == m_vecPosSkill.end())
        return nullptr;
      return &(*v);*/
    }
    else
    {
      for (auto &v : VEC_SHORTCUT_EXTEND)
      {
        SPosData* pData = getShortcut(v, pos);
        if (pData != nullptr && pData->dwID != 0)
          return pData;
      }
      /*auto v = find_if(m_vecExtendPosSkill.begin(), m_vecExtendPosSkill.end(), [&](const SPosData& r) ->bool{
          return r.dwPos == pos;
          });
      if (v == m_vecExtendPosSkill.end())
        return nullptr;
      return &(*v);*/
    }
  }

  return nullptr;
}

void FighterSkill::addPosUpdate(DWORD skillid, DWORD sourceid)
{
  if (m_pFighter == nullptr || m_pFighter->getUser() == nullptr)
    return;
  if (skillid == 0)
    return;

  SceneUser* pUser = m_pFighter->getUser();
  SceneFighter* pNoviceFighter = pUser->getFighter(EPROFESSION_NOVICE);
  SSkillItem* pItem = nullptr;
  for (auto v = m_vecSkillData.begin(); v != m_vecSkillData.end(); ++v)
  {
    pItem = v->getSkillItem(skillid, sourceid);
    if (pItem != nullptr)
      break;
  }
  if (pItem == nullptr)
  {
    if (pNoviceFighter != nullptr)
    {
      SSkillData* pData = pNoviceFighter->getSkill().getSkillData(EPROFESSION_NOVICE);
      if (pData != nullptr)
        pItem = pData->getSkillItem(skillid, sourceid);
    }
  }
  if (pItem == nullptr)
  {
    XERR << "[SkillUpdate], 找不到技能id=" << skillid << "sourceid=" << sourceid << XEND;
    return;
  }
  addupdate(pItem->eProfession, skillid, sourceid);
}

void FighterSkill::forceEnableSkill(DWORD id)
{
  if (m_pFighter == nullptr || m_pFighter->getUser() == nullptr || m_pFighter->getUser()->getRoleBaseCFG() == nullptr)
    return;

  SceneFighter* pNoviceFighter = m_pFighter->getUser()->getFighter(EPROFESSION_NOVICE);
  if (pNoviceFighter != nullptr && pNoviceFighter != m_pFighter)
  {
    pNoviceFighter->getSkill().forceEnableSkill(id);
    pNoviceFighter->getSkill().update(now());
  }

  const SRoleBaseCFG* pCFG = m_pFighter->getRoleCFG();
  if (pCFG == nullptr)
    return;
  SceneUser* pUser = m_pFighter->getUser();
  for (auto v = pCFG->vecEnableSkill.begin(); v != pCFG->vecEnableSkill.end(); ++v)
  {
    SSkillData* pData = getSkillData(v->first);
    if (pData == nullptr)
      continue;

    for (auto o = v->second.begin(); o != v->second.end(); ++o)
    {
      if ((*o) != id)
        continue;

     // get skill and add if noexist
      SSkillItem* pItem = pData->getSkillByType(*o);
      if (pItem == nullptr)
      {
        const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(*o);
        if (pSkillCFG == nullptr)
        {
          XERR << "[角色发放技能]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "未找到 pro:" << v->first << "skillid:" << *o << "技能" << XEND;
          continue;
        }

        SSkillItem stItem;
        stItem.dwID = *o;
        stItem.eProfession = v->first;
        stItem.pSkillCFG = pSkillCFG;
        stItem.consume.nexttime = now();
        if (stItem.eProfession == EPROFESSION_NOVICE)
        {
          stItem.bLearn = true;
          stItem.bActive = false;
          if (stItem.pSkillCFG->getSkillType() != ESKILLTYPE_PASSIVE)
          {
            //setValidPos(*o, 0);
            equipSkill(ESKILLSHORTCUT_NORMAL, *o, 0);
            if (pSkillCFG->canEquipAuto() && pCFG->normalSkill == *o)
              //setValidAutoPos(*o, 0);
              equipSkill(ESKILLSHORTCUT_AUTO, *o, 0);
          }
          if (stItem.pSkillCFG->haveDirectBuff())
          {
            m_pFighter->getUser()->m_oBuff.addSkillBuff(*o);
          }
        }
        pData->vecSkillItem.push_back(stItem);
        addupdate(v->first, *o);
        pItem = pData->getSkillItem(*o);
      }
      if (pItem == nullptr)
        continue;
      if (pItem->eProfession != EPROFESSION_NOVICE)
        pItem->bActive = true;

      XLOG << "[玩家-强制发放技能], 玩家:" << m_pFighter->getUser()->name << m_pFighter->getUser()->id << "技能:" << id << XEND;
      addupdate(pItem->eProfession, id);
    }
  }
}

bool FighterSkill::isSkillFamilyEquiped(DWORD familyid) const
{
  /*for (auto &v : m_vecPosSkill)
  {
    if (v.dwID / ONE_THOUSAND == familyid)
      return true;
  }
  for (auto &v : m_vecExtendPosSkill)
  {
    if (v.dwID / ONE_THOUSAND == familyid)
      return true;
  }
  for (auto &v : m_vecAutoPosSkill)
  {
    if (v.dwID / ONE_THOUSAND == familyid)
    {
      if (isAutoPosValid(v.dwPos))
        return true;
    }
  }*/

  for (DWORD d = ESKILLSHORTCUT_MIN + 1; d < ESKILLSHORTCUT_MAX; ++d)
  {
    auto m = m_mapShortcut.find(static_cast<ESkillShortcut>(d));
    if (m == m_mapShortcut.end())
      continue;

    const TVecPosSkill& vecPos = m->second;
    for (auto &v : vecPos)
    {
      if (v.dwID / ONE_THOUSAND == familyid)
        return d == ESKILLSHORTCUT_AUTO ? isAutoPosValid(v.dwPos) : true;
    }
  }
  return false;
}

bool FighterSkill::isSkillEquiped(DWORD id) const
{
  DWORD oriid = 0;
  if (!m_mapTempReplaceRecord.empty())
  {
    auto it = m_mapTempReplaceRecord.find(id / ONE_THOUSAND);
    if (it != m_mapTempReplaceRecord.end())
      oriid = it->second * ONE_THOUSAND + id % ONE_THOUSAND;
  }
  DWORD tmpid = getOrinalSkill(id);
  if (tmpid) oriid = tmpid;

  /*for (auto &v : m_vecPosSkill)
  {
    if (v.dwID == id || v.dwID == oriid)
      return true;
    if (v.dwID / ONE_THOUSAND == id / ONE_THOUSAND) // 有额外等级
      return true;
  }
  for (auto &v : m_vecExtendPosSkill)
  {
    if (v.dwID == id || v.dwID == oriid)
      return true;
    if (v.dwID / ONE_THOUSAND == id / ONE_THOUSAND) // 有额外等级
      return true;
  }
  for (auto &v : m_vecAutoPosSkill)
  {
    if (v.dwID == id || v.dwID == oriid || v.dwID / ONE_THOUSAND == id / ONE_THOUSAND)
    {
      if (isAutoPosValid(v.dwPos))
        return true;
    }
  }*/
  for (auto &m : m_mapShortcut)
  {
    for (auto &v : m.second)
    {
      if (v.dwID == id || v.dwID == oriid || v.dwID / ONE_THOUSAND == id / ONE_THOUSAND)
        return m.first == ESKILLSHORTCUT_AUTO ? isAutoPosValid(v.dwPos) : true;
    }
  }

  SceneFighter* pNoviceFighter = m_pFighter->getUser()->getFighter(EPROFESSION_NOVICE);
  if (pNoviceFighter != m_pFighter)
  {
    if (pNoviceFighter == nullptr)
      return false;
    if (pNoviceFighter->getSkill().isSkillEquiped(id))
      return true;
  }

  const SNewRoleCFG& rCFG = MiscConfig::getMe().getNewRoleCFG();
  if (id == rCFG.dwCollectSkill || id == rCFG.dwTransSkill || id == rCFG.dwFlashSkill)// || id == rCFG.dwRepairSkill)
    return true;
  if (m_setTempIDs.find(id) != m_setTempIDs.end())
    return true;

  return false;
}

bool FighterSkill::isInSlot(DWORD skillid, DWORD sourceid) const
{
  for (auto &m : m_mapShortcut)
  {
    auto v = find_if(m.second.begin(), m.second.end(), [&](const SPosData& rData) -> bool{
      return rData.dwID == skillid && rData.dwSourceid == sourceid;
    });
    if (v != m.second.end())
      return true;
  }
  return false;
  /*auto findok = [&](const TVecPosSkill& vecskill) -> bool
  {
    for (auto &v : vecskill)
    {
      if (v.dwID == skillid && v.dwSourceid == sourceid)
        return true;
    }
    return false;
  };
  if (findok(m_vecPosSkill))
    return true;
  if (findok(m_vecAutoPosSkill))
    return true;
  if (findok(m_vecExtendPosSkill))
    return true;
  return false;*/
}

bool FighterSkill::hasSkill(DWORD id, DWORD sourceid)
{
  for (auto v = m_vecSkillData.begin(); v != m_vecSkillData.end(); ++v)
  {
    if (v->getSkillItem(id, sourceid) != nullptr)
      return true;
  }

  SceneFighter* pNoviceFighter = m_pFighter->getUser()->getFighter(EPROFESSION_NOVICE);
  if (pNoviceFighter != m_pFighter)
  {
    if (pNoviceFighter == nullptr)
      return false;
    FighterSkill& rSkill = pNoviceFighter->getSkill();
    for (auto v = rSkill.m_vecSkillData.begin(); v != rSkill.m_vecSkillData.end(); ++v)
    {
      if (v->getSkillItem(id, sourceid) != nullptr)
        return true;
    }
  }

  return m_setTempIDs.find(id) != m_setTempIDs.end();
}

bool FighterSkill::replaceSkill(DWORD oldid, DWORD newid)
{
  //check NEW skill illegal
  if (oldid == newid)
    return false;
  const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(newid);
  if (pSkill == nullptr)
    return false;

  SceneUser* pUser = m_pFighter ? m_pFighter->getUser() : nullptr;
  if (!pUser)
    return false;

  SceneFighter* pNoviceFighter = pUser->getFighter(EPROFESSION_NOVICE);
  if (pNoviceFighter == nullptr)
    return false;
  SSkillData* pNoviceData = pNoviceFighter->getSkill().getSkillData(EPROFESSION_NOVICE);
  if (pNoviceData == nullptr)
    return false;

  auto it = find_if(pNoviceData->vecSkillItem.begin(), pNoviceData->vecSkillItem.end(), [&](const SSkillItem& r) -> bool{
      return r.dwID == oldid;
      });
  if (it == pNoviceData->vecSkillItem.end())
    return false;

  /*DWORD pos = getPos(oldid, 0);
  DWORD autopos = getAutoPos(oldid, 0);
  DWORD extendpos = getExtendPos(oldid, 0);
  if (pos == 0 && autopos == 0 && extendpos == 0)
  {
    pos = getPos(newid, 0);
    autopos = getAutoPos(newid, 0);
    extendpos = getExtendPos(newid, 0);
  }*/

  removeSkill(oldid, 0, ESOURCE_MIN);
  addSkill(newid, 0, ESOURCE_MIN);

  for (DWORD d = ESKILLSHORTCUT_MIN + 1; d < ESKILLSHORTCUT_MAX; ++d)
  {
    auto m = m_mapShortcut.find(static_cast<ESkillShortcut>(d));
    if (m == m_mapShortcut.end())
      continue;
    TVecPosSkill& vecSkill = m->second;
    for (auto &v : vecSkill)
    {
      if (v.dwID == oldid)
        v.dwID = newid;
    }
  }

  /*setPos(newid, 0, pos);
  setAutoPos(newid, 0, autopos);
  setExtendPos(newid, 0, extendpos);*/
  addupdate(EPROFESSION_NOVICE, newid, 0);

  XLOG << "[玩家-技能替换], 玩家:" << pUser->name << pUser->id << "老的技能:" << oldid << "新的技能:" << newid << XEND;

  return true;
}

void FighterSkill::replaceNormalSkill(DWORD dwNewSkill)
{
  if (m_pFighter == nullptr)
    return;
  SceneUser* pUser = m_pFighter->getUser();
  if (pUser == nullptr)
    return;

  const SRoleBaseCFG* pCFG = pUser->getRoleBaseCFG();
  if (pCFG == nullptr)
    return;

  DWORD normalskill = pCFG->normalSkill;
  if (normalskill == dwNewSkill)
    return;

  // 判断新加技能是否需要加到自动攻击技能栏(有待优化)
  DWORD oldReplaceNormalSkillID = m_dwReplaceNormalSkillID;
  m_dwReplaceNormalSkillID = dwNewSkill;
  if (replaceSkill(normalskill, dwNewSkill) == false)
  {
    // 替换失败 还原技能
    m_dwReplaceNormalSkillID = oldReplaceNormalSkillID;
    return;
  }

  XLOG << "[玩家-普攻变化], 玩家:" << pUser->name << pUser->id << "替换前技能:" << normalskill << "替换后技能:" << dwNewSkill << XEND;

  pUser->setDataMark(EUSERDATATYPE_NORMAL_SKILL);
  pUser->refreshDataAtonce();
}

void FighterSkill::restoreNormalSkill()
{
  if (m_pFighter == nullptr)
    return;
  SceneUser* pUser = m_pFighter->getUser();
  if (pUser == nullptr)
    return;

  const SRoleBaseCFG* pCFG = pUser->getRoleBaseCFG();
  if (pCFG == nullptr)
    return;

  DWORD normalskill = pCFG->normalSkill;
  if (m_dwReplaceNormalSkillID == 0 || normalskill == m_dwReplaceNormalSkillID)
    return;

  // 判断新加技能是否需要加到自动攻击技能栏(有待优化)
  DWORD oldReplaceNormalSkillID = m_dwReplaceNormalSkillID;
  m_dwReplaceNormalSkillID = 0;
  if (replaceSkill(oldReplaceNormalSkillID, normalskill) == false)
  {
    // 替换失败 还原技能
    m_dwReplaceNormalSkillID = oldReplaceNormalSkillID;
    return;
  }

  XLOG << "[玩家-普攻恢复], 玩家:" << pUser->name << pUser->id << "替换前技能:" << m_dwReplaceNormalSkillID << "替换后技能:" << normalskill << XEND;

  pUser->setDataMark(EUSERDATATYPE_NORMAL_SKILL);
  pUser->refreshDataAtonce();
}

void SSpecSkillInfo::toData(SpecSkillInfo* pInfo)
{
  if (pInfo == nullptr)
    return;
  pInfo->set_id(dwID);
  for (auto &m : mapAttrValue)
  {
    UserAttr* pAttr = pInfo->add_attrs();
    if (pAttr == nullptr)
      continue;
    pAttr->set_type(m.first);
    if (RoleDataConfig::getMe().needSyncPercent(m.first))
      pAttr->set_value(floor(m.second * ONE_THOUSAND + 0.5));
    else
      pAttr->set_value(m.second);
  }

  for (auto &m : mapItem2NumAndPer)
  {
    SkillCost* pCost = pInfo->add_cost();
    if (pCost == nullptr)
      continue;
    pCost->set_itemid(m.first);
    pCost->set_changenum(m.second.first);
    pCost->set_changeper(m.second.second * ONE_THOUSAND);
  }

  if (fChRange)
    pInfo->set_changerange(fChRange * ONE_THOUSAND);
  if (intChTarCount)
    pInfo->set_changenum(intChTarCount);
  if (fChReady)
    pInfo->set_changeready(fChReady * ONE_THOUSAND);
  if(bNeedNoItem)
    pInfo->set_neednoitem(true);
}

void SSpecSkillInfo::clearData()
{
  for (auto &m : mapAttrValue)
    m.second = 0;
  for (auto &m : mapItem2NumAndPer)
  {
    m.second.first = 0;
    m.second.second = 0;
  }
  intChCount = 0;
  fChRange = 0;
  fChDurationPer = 0;
  intChTarCount = 0;
  fChReady = 0;
  intChLimitCnt = 0;
  bNeedNoItem = false;
}

bool SSpecSkillInfo::checkClear()
{
  for (auto &m : mapAttrValue)
  {
    if (m.second)
      return false;
  }
  for (auto &m : mapItem2NumAndPer)
  {
    if (m.second.first)
      return false;
    if (m.second.second)
      return false;
  }
  if (intChCount || fChRange || fChDurationPer || intChTarCount || fChReady || intChLimitCnt || bNeedNoItem)
    return false;

  return true;
}

// 获取影响技能的参数(不包括对有所有技能影响的buff)
SSpecSkillInfo* FighterSkill::getSpecWithoutGlobal(DWORD skillid)
{
  if(m_mapSpecSkillInfo.empty())
  {
    return nullptr;
  }
  
  DWORD familyID = skillid / ONE_THOUSAND;

  auto it = m_mapSpecSkillInfo.find(familyID);
  if( it == m_mapSpecSkillInfo.end())
  {
    return nullptr;
  }
  
  return &(it->second);
}

float FighterSkill::getSkillAttr(DWORD skillid, EAttrType etype)
{
  float sum = 0.0;
  SSpecSkillInfo* pInfo = getSpecWithoutGlobal(skillid);

  if(pInfo != nullptr)
  {
    auto it = pInfo->mapAttrValue.find(etype);
    if (it != pInfo->mapAttrValue.end())
       sum += it->second;
  }

  // 全局的影响
  auto allskillIt = m_sAllSkillInfo.mapAttrValue.find(etype);
  if(allskillIt != m_sAllSkillInfo.mapAttrValue.end())
    sum += allskillIt->second;

  return sum;
}

pair<int, float> FighterSkill::getCostItemInfo(DWORD skillid, DWORD itemid)
{
  static const pair<int, float> emptycost;

  pair<int, float> sumcost;
  
  SSpecSkillInfo* pInfo = getSpecWithoutGlobal(skillid);
  
  if(pInfo != nullptr)
  {
    if(pInfo->bNeedNoItem)
      return emptycost;

    auto it = pInfo->mapItem2NumAndPer.find(itemid);
    if (it != pInfo->mapItem2NumAndPer.end())
    {
      sumcost.first += it->second.first;
      sumcost.second += it->second.second;
    }
  }

  // 全局的影响
  if(m_sAllSkillInfo.bNeedNoItem)
    return emptycost;
  
  auto allskillIt = m_sAllSkillInfo.mapItem2NumAndPer.find(itemid);
  if(allskillIt != m_sAllSkillInfo.mapItem2NumAndPer.end())
  {
    sumcost.first += allskillIt->second.first;
    sumcost.second += allskillIt->second.second;
  }
  return sumcost;
}

float FighterSkill::getDurationPer(DWORD skillid)
{
  float sum = 0.0;
  
  SSpecSkillInfo* pInfo = getSpecWithoutGlobal(skillid);
  if(pInfo != nullptr)
    sum += pInfo->fChDurationPer;
  
  // 全局的影响
  sum += m_sAllSkillInfo.fChDurationPer;

  return sum;
}

float FighterSkill::getChangeRange(DWORD skillid)
{
  float sum = 0.0;

  SSpecSkillInfo* pInfo = getSpecWithoutGlobal(skillid);
  if(pInfo != nullptr)
    sum += pInfo->fChRange;

  // 全局的影响
  sum += m_sAllSkillInfo.fChRange;

  return sum;
}

float FighterSkill::getChangeReady(DWORD skillid)
{
  float sum = 0.0;

  SSpecSkillInfo* pInfo = getSpecWithoutGlobal(skillid);
  if(pInfo != nullptr)
    sum += pInfo->fChReady;

  // 全局的影响
  sum += m_sAllSkillInfo.fChReady;

  return sum;
}

int FighterSkill::getChangeCount(DWORD skillid)
{
  int sum = 0;

  SSpecSkillInfo* pInfo = getSpecWithoutGlobal(skillid);
  if(pInfo != nullptr)
    sum += pInfo->intChCount;

  // 全局的影响
  sum += m_sAllSkillInfo.intChCount;

  return sum;
}

int FighterSkill::getChangeTarCount(DWORD skillid)
{
  int sum = 0;

  SSpecSkillInfo* pInfo = getSpecWithoutGlobal(skillid);
  if(pInfo != nullptr)
    sum += pInfo->intChTarCount;

  // 全局的影响
  sum += m_sAllSkillInfo.intChTarCount;

  return sum;
}

int FighterSkill::getLimitCount(DWORD skillid)
{
  int sum = 0;

  SSpecSkillInfo* pInfo = getSpecWithoutGlobal(skillid);
  if(pInfo != nullptr)
    sum += pInfo->intChLimitCnt;

  // 全局的影响
  sum += m_sAllSkillInfo.intChLimitCnt;

  return sum;
}

void FighterSkill::sendSpecSkillInfo()
{
  if (!m_setSpecSkillUpdates.empty())
  {
    updateSpecSkill();
    return;
  }

  if (m_mapSpecSkillInfo.empty())
    return;
  UpSkillInfoSkillCmd message;
  for (auto &m : m_mapSpecSkillInfo)
  {
    SpecSkillInfo* pInfo = message.add_specinfo();
    if (pInfo == nullptr)
      continue;
    m.second.toData(pInfo);
  }

  // 对全部技能生效的spec
  if( !m_sAllSkillInfo.checkClear() )
  {
    SpecSkillInfo* pInfoAll = message.mutable_allskillinfo();
    if( pInfoAll != nullptr )
    {
      pInfoAll->set_id(0);
      m_sAllSkillInfo.toData(pInfoAll);
    }
  }

  PROTOBUF(message, send, len);
  if (m_pFighter && m_pFighter->getUser())
    m_pFighter->getUser()->sendCmdToMe(send, len);
}

void FighterSkill::collectSpecSkill(DWORD familySkillID)
{
  SceneUser* user = m_pFighter ? m_pFighter->getUser() : nullptr;
  if (user == nullptr)
    return;

  SSpecSkillInfo& info = m_mapSpecSkillInfo[familySkillID];
  info.clearData();
  info.dwID = familySkillID;
  user->m_oBuff.getSpecSkillInfo(familySkillID, info);
}

void FighterSkill::updateAllSkill()
{
  SceneUser* user = m_pFighter ? m_pFighter->getUser() : nullptr;
  if (user == nullptr)
    return;

  m_sAllSkillInfo.clearData();

  user->m_oBuff.getAllSkillSpec(m_sAllSkillInfo);
}

void FighterSkill::updateSpecSkill()
{
  if (m_setSpecSkillUpdates.empty())
    return;

  for (auto &s : m_setSpecSkillUpdates)
  {
    if(s != 0)
      collectSpecSkill(s);
    else
    {
      updateAllSkill();
    }
  }

  m_setSpecSkillUpdates.clear();

  // send to client
  UpSkillInfoSkillCmd message;
  for (auto &m : m_mapSpecSkillInfo)
  {
    SpecSkillInfo* pInfo = message.add_specinfo();
    if (pInfo == nullptr)
      continue;
    m.second.toData(pInfo);
  }
  // 所有技能的影响
  if( !m_sAllSkillInfo.checkClear() )
  {
    SpecSkillInfo* pInfoAll = message.mutable_allskillinfo();
    if( pInfoAll != nullptr )
    {
      pInfoAll->set_id(0);
      m_sAllSkillInfo.toData(pInfoAll);
    }
  }
  
  PROTOBUF(message, send, len);
  if (m_pFighter && m_pFighter->getUser())
    m_pFighter->getUser()->sendCmdToMe(send, len);

  for (auto m = m_mapSpecSkillInfo.begin(); m != m_mapSpecSkillInfo.end(); )
  {
    if (m->second.checkClear())
    {
      m = m_mapSpecSkillInfo.erase(m);
      continue;
    }
    ++m;
  }
}

void FighterSkill::switchRune(DWORD skillid, bool isopen)
{
  SceneUser* pUser = m_pFighter->getUser();
  if (pUser == nullptr)
    return;

  DWORD offRuneID = 0;
  DWORD openRuneID = 0;

  for (auto &v : m_vecSkillData)
  {
    if (v.eProfession == EPROFESSION_NOVICE)
      continue;
    auto s = find_if(v.vecSkillItem.begin(), v.vecSkillItem.end(), [&](const SSkillItem& r)->bool {
        return r.dwID == skillid;
    });
    if (s == v.vecSkillItem.end())
      continue;
    if (isopen == s->bRuneOpened)
      return;
    s->bRuneOpened = isopen;

    if (s->dwRuneSpecID)
    {
      if (isopen == false)
      {
        onOffRuneSpecID(s->dwRuneSpecID);
        offRuneID = s->dwRuneSpecID;
      }
      else
      {
        onSelectRuneSpecID(s->dwRuneSpecID);
        openRuneID = s->dwRuneSpecID;
      }
    }

    addupdate(v.eProfession, skillid);
    XLOG << "[技能-选择携带星盘点], 玩家:" << pUser->name << pUser->id << "技能:" << skillid << "符文开关:" << isopen << XEND;
    break;
  }
  if (offRuneID) onClientChangeRune(offRuneID, false);
  if (openRuneID) onClientChangeRune(openRuneID, true);
  // client need refresh atonce
  update(now());
}

void FighterSkill::selectRuneSpecID(DWORD familySkillID, DWORD runespecid)
{
  SceneUser* pUser = m_pFighter->getUser();
  if (pUser == nullptr)
    return;

  // check can select
  if (pUser->getAstrolabes().isEffectUnlock(runespecid) == false)
    return;
  const SRuneSpecCFG* pRuneCFG = AstrolabeConfig::getMe().getRuneSpecCFG(runespecid);
  if (pRuneCFG == nullptr || pRuneCFG->setSkillIDs.find(familySkillID) == pRuneCFG->setSkillIDs.end())
    return;

  DWORD offRuneID = 0;
  DWORD openRuneID = 0;
  for (auto &v : m_vecSkillData)
  {
    if (v.eProfession == EPROFESSION_NOVICE)
      continue;
    auto s = find_if(v.vecSkillItem.begin(), v.vecSkillItem.end(), [&](const SSkillItem& r)->bool {
        return r.dwID / ONE_THOUSAND == familySkillID;
    });
    if (s == v.vecSkillItem.end() || s->dwRuneSpecID == runespecid)
      continue;
    if (s->bRuneOpened)
    {
      if (s->dwRuneSpecID)
      {
        onOffRuneSpecID(s->dwRuneSpecID);
        offRuneID = s->dwRuneSpecID;
      }
      onSelectRuneSpecID(runespecid);
      openRuneID = runespecid;
    }
    s->dwRuneSpecID = runespecid;
    addupdate(v.eProfession, s->dwID);
    XLOG << "[技能-选择携带星盘点], 玩家:" << pUser->name << pUser->id << "技能:" << s->dwID << "星盘效果ID:" << runespecid << XEND;
    break;
  }
  // client need refresh atonce
  if (offRuneID) onClientChangeRune(offRuneID, false);
  if (openRuneID) onClientChangeRune(openRuneID, true);
  update(now());
}

void FighterSkill::onSelectRuneSpecID(DWORD runespecid)
{
  SceneUser* pUser = m_pFighter->getUser();
  if (pUser == nullptr)
    return;

  const SRuneSpecCFG* pRuneCFG = AstrolabeConfig::getMe().getRuneSpecCFG(runespecid);
  if (pRuneCFG == nullptr)
    return;

  if (pRuneCFG->setBuffIDs.empty() == false)
  {
    for (auto &s : pRuneCFG->setBuffIDs)
    {
      pUser->m_oBuff.add(s);
    }
  }
}

void FighterSkill::onOffRuneSpecID(DWORD runespecid)
{
  SceneUser* pUser = m_pFighter->getUser();
  if (pUser == nullptr)
    return;

  const SRuneSpecCFG* pRuneCFG = AstrolabeConfig::getMe().getRuneSpecCFG(runespecid);
  if (pRuneCFG == nullptr)
    return;

  if (pRuneCFG->setBuffIDs.empty() == false)
  {
    for (auto &s : pRuneCFG->setBuffIDs)
    {
      pUser->m_oBuff.del(s);
    }
  }
}

bool FighterSkill::isRuneSpecSelected(DWORD runespecid)
{
  for (auto &v : m_vecSkillData)
  {
    if (v.eProfession == EPROFESSION_NOVICE)
      continue;
    for (auto &s : v.vecSkillItem)
    {
      if (s.dwRuneSpecID == runespecid && s.bRuneOpened)
        return true;
    }
  }
  return false;
}

DWORD FighterSkill::getRuneSelectID(DWORD familySkillID)
{
  for (auto &v : m_vecSkillData)
  {
    if (v.eProfession == EPROFESSION_NOVICE)
      continue;
    auto s = find_if(v.vecSkillItem.begin(), v.vecSkillItem.end(), [&](const SSkillItem& r)->bool {
        return r.dwID / ONE_THOUSAND == familySkillID;
    });
    if (s == v.vecSkillItem.end())
      continue;
    if (s->dwRuneSpecID != 0 && s->bRuneOpened)
      return s->dwRuneSpecID;
    else
      return 0;
  }
  return 0;
}

void FighterSkill::onRuneReset(DWORD runespecid)
{
  for (auto &v : m_vecSkillData)
  {
    if (v.eProfession == EPROFESSION_NOVICE)
      continue;
    for (auto &s : v.vecSkillItem)
    {
      if (s.dwRuneSpecID != runespecid)
        continue;
      s.dwRuneSpecID = 0;
      if (s.bRuneOpened)
        onOffRuneSpecID(runespecid);
      addupdate(v.eProfession, s.dwID);
    }
  }
}

void FighterSkill::validMonthCardSkillPos()
{  
  if (m_pFighter == nullptr || m_pFighter->getUser() == nullptr)
    return;
  SceneUser* user = m_pFighter->getUser();
  if (!user)
    return;

  if (m_bMonthCardSkillValid)
    return;
  m_bMonthCardSkillValid = true;
  
  /*SkillValidPos cmd;
  toClient(cmd);
  PROTOBUF(cmd, send, len);
  m_pFighter->getUser()->sendCmdToMe(send, len);*/
  sendValidPos();
  XLOG << "[技能自动格子-加1],月卡有效，玩家:" << user->name << user->id << "当前格子数量:" << m_dwAutoMaxPos << XEND;
}

void FighterSkill::invalidMonthCardSkillPos()
{
  if (!m_pFighter)
    return;
  SceneUser* user = m_pFighter->getUser();
  if (!user)
    return;
  if (!m_bMonthCardSkillValid)
    return;
  m_bMonthCardSkillValid = false;

  /*SkillValidPos cmd;
  toClient(cmd);
  PROTOBUF(cmd, send, len);
  user->sendCmdToMe(send, len);*/
  sendValidPos();

  XLOG << "[技能自动格子-减1],月卡失效，玩家:" << user->name << user->id << "当前格子数量:" << m_dwAutoMaxPos << XEND;
}

//判断位置是否正确，不判断月卡过期不过期
bool FighterSkill::isRightAutoPos(DWORD pos)const 
{
  if (pos == MONTH_CARD_SKILL_POS)
  {
    return true;
  }

  if (pos > m_dwAutoMaxPos)
    return false;
  return true;
}

//判断位置是否有效
bool FighterSkill::isAutoPosValid(DWORD pos) const 
{
  if (pos == 0)
    return false;
  if (pos == MONTH_CARD_SKILL_POS)
    return m_bMonthCardSkillValid;

  if (pos > m_dwAutoMaxPos)
    return false;
  return true;
}

void FighterSkill::setReplaceSkill(DWORD oldFamilyID, DWORD newFamilyID)
{
  for (auto &v : m_vecSkillData)
  {
    auto s = find_if(v.vecSkillItem.begin(), v.vecSkillItem.end(), [&](const SSkillItem& r)->bool{
      return r.dwID / ONE_THOUSAND == oldFamilyID;
    });
    if (s == v.vecSkillItem.end())
      continue;
    s->dwReplaceID = newFamilyID * ONE_THOUSAND + s->dwID % ONE_THOUSAND;
    addupdate(v.eProfession, s->dwID, s->dwSourceid);
    m_mapTempReplaceRecord[newFamilyID] = oldFamilyID;
  }
}

void FighterSkill::delReplaceSkill(DWORD oldFamilyID, DWORD newFamilyID)
{
  for (auto &v : m_vecSkillData)
  {
    auto s = find_if(v.vecSkillItem.begin(), v.vecSkillItem.end(), [&](const SSkillItem& r)->bool{
      return r.dwID / ONE_THOUSAND == oldFamilyID && r.dwReplaceID / ONE_THOUSAND == newFamilyID;
    });
    if (s == v.vecSkillItem.end())
      continue;
    s->dwReplaceID = 0;
    addupdate(v.eProfession, s->dwID, s->dwSourceid);

    auto it = m_mapTempReplaceRecord.find(newFamilyID);
    if (it != m_mapTempReplaceRecord.end())
      m_mapTempReplaceRecord.erase(it);
  }
}

DWORD FighterSkill::getReplaceSkill(DWORD dwFamilySkill)
{
  DWORD dwReplacedID = 0;
  for (auto &v : m_vecSkillData)
  {
    auto s = find_if(v.vecSkillItem.begin(), v.vecSkillItem.end(), [&](const SSkillItem& r)->bool{
        return r.dwID / ONE_THOUSAND == dwFamilySkill;
        });
    if (s == v.vecSkillItem.end())
      continue;
    dwReplacedID = s->dwReplaceID;
  }

  if (dwReplacedID == 0 && m_pFighter && m_pFighter->getProfession() != EPROFESSION_NOVICE && m_pFighter->getUser())
  {   
    SceneFighter* pNoviceFighter = m_pFighter->getUser()->getFighter(EPROFESSION_NOVICE);
    if (pNoviceFighter == nullptr)
      return 0;
    dwReplacedID = pNoviceFighter->getSkill().getReplaceSkill(dwFamilySkill);
  }
  return dwReplacedID;
}

void FighterSkill::addExtraSkillLv(DWORD familyID, DWORD lv)
{
  for (auto &v : m_vecSkillData)
  {
    auto s = find_if(v.vecSkillItem.begin(), v.vecSkillItem.end(), [&](const SSkillItem& r)->bool{
      return r.dwID / ONE_THOUSAND == familyID;
    });
    if (s == v.vecSkillItem.end())
      continue;
    s->dwExtraLv += lv;
    addupdate(v.eProfession, s->dwID, s->dwSourceid);
    return;
  }

  if (m_pFighter && m_pFighter->getProfession() != EPROFESSION_NOVICE && m_pFighter->getUser())
  {
    SceneFighter* pNoviceFighter = m_pFighter->getUser()->getFighter(EPROFESSION_NOVICE);
    if (pNoviceFighter != nullptr && pNoviceFighter != m_pFighter)
      pNoviceFighter->getSkill().addExtraSkillLv(familyID, lv);
  }
}

void FighterSkill::decExtraSkillLv(DWORD familyID, DWORD lv)
{
  for (auto &v : m_vecSkillData)
  {
    auto s = find_if(v.vecSkillItem.begin(), v.vecSkillItem.end(), [&](const SSkillItem& r)->bool{
      return r.dwID / ONE_THOUSAND == familyID;
    });
    if (s == v.vecSkillItem.end())
      continue;
    s->dwExtraLv = (s->dwExtraLv > lv ? s->dwExtraLv - lv : 0);
    addupdate(v.eProfession, s->dwID, s->dwSourceid);
    return;
  }

  if (m_pFighter && m_pFighter->getProfession() != EPROFESSION_NOVICE && m_pFighter->getUser())
  {
    SceneFighter* pNoviceFighter = m_pFighter->getUser()->getFighter(EPROFESSION_NOVICE);
    if (pNoviceFighter != nullptr && pNoviceFighter != m_pFighter)
      pNoviceFighter->getSkill().decExtraSkillLv(familyID, lv);
  }
}

void FighterSkill::resetWeddingSkill(bool bMarry)
{
  if (m_pFighter == nullptr)
    return;
  SceneUser* pUser = m_pFighter->getUser();
  if (pUser == nullptr)
    return;

  const SWeddingMiscCFG& rCFG = MiscConfig::getMe().getWeddingMiscCFG();

  for (auto &s : rCFG.setMarrySkills)
  {
    DWORD dwLv = getSkillLv(s);
    if (bMarry)
    {
      if (dwLv == 0)
      {
        if (addSkill(s, 0, ESOURCE_NORMAL) == true)
          XLOG << "[技能-结婚技能]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "婚姻关系为" << bMarry << "添加技能" << s << XEND;
      }
    }
    else
    {
      if (dwLv != 0)
      {
        if (removeSkill(s, 0, ESOURCE_NORMAL) == true)
          XLOG << "[技能-结婚技能]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "婚姻关系为" << bMarry << "移除技能" << s << XEND;
      }
    }
  }

  DWORD dwWelcomeLv = getSkillLv(SKILL_WEDDING_WELCOME);
  if (bMarry)
  {
    if (dwWelcomeLv == 0 && pUser->getQuest().isSubmit(QUEST_WEDDING_WELCOME) == true && addSkill(SKILL_WEDDING_WELCOME, SKILL_SOURCEID, ESOURCE_NORMAL) == true)
      XLOG << "[技能-结婚技能]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "婚姻关系为" << bMarry << "添加技能" << SKILL_WEDDING_WELCOME << XEND;
  }
  else
  {
    if (dwWelcomeLv != 0 && removeSkill(SKILL_WEDDING_WELCOME, SKILL_SOURCEID, ESOURCE_NORMAL) == true)
      XLOG << "[技能-结婚技能]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "婚姻关系为" << bMarry << "移除技能" << SKILL_WEDDING_WELCOME << XEND;
  }
  XDBG << "[技能-结婚技能]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "婚姻关系为" << bMarry << "刷新结婚技能" << XEND;

  // food skill
  const SFoodMiscCFG& rFoodCFG = MiscConfig::getMe().getFoodCfg();
  if (rFoodCFG.dwSkillId != 0)
  {
    DWORD dwLv = getSkillLv(rFoodCFG.dwSkillId);
    bool bOpen = pUser->getMenu().isOpen(EMENUID_FOOD);
    if (bOpen && dwLv == 0)
      addSkill(rFoodCFG.dwSkillId, 0, ESOURCE_SHOP);
    if (!bOpen && dwLv != 0)
      removeSkill(rFoodCFG.dwSkillId, 0, ESOURCE_SHOP);
  }
}

void FighterSkill::onClientChangeRune(DWORD dwRuneSpecID, bool bOpen)
{
  if (dwRuneSpecID == 0)
    return;
  // same runeSpecID open/stop synchronously
  for (auto &v : m_vecSkillData)
  {
    if (v.eProfession == EPROFESSION_NOVICE)
      continue;
    for (auto &s : v.vecSkillItem)
    {
      if (s.dwRuneSpecID != dwRuneSpecID)
        continue;
      if (s.bRuneOpened == bOpen)
        continue;

      s.bRuneOpened = bOpen;
      if (bOpen)
        onSelectRuneSpecID(dwRuneSpecID);
      else
        onOffRuneSpecID(dwRuneSpecID);
      addupdate(v.eProfession, s.dwID);
    }
  }
}

bool FighterSkill::changeSkill(DWORD oldskillid, DWORD newskillid)
{
  if (m_pFighter && m_pFighter->getProfession() != EPROFESSION_NOVICE && m_pFighter->getUser())
  {
    SceneFighter* pNoviceFighter = m_pFighter->getUser()->getFighter(EPROFESSION_NOVICE);
    if (pNoviceFighter == nullptr)
      return false;
    if (pNoviceFighter->getSkill().changeSkill(oldskillid, newskillid))
    {
      return true;
    }
  }

  for (auto &v : m_vecSkillData)
  {
    auto s = find_if(v.vecSkillItem.begin(), v.vecSkillItem.end(), [&](const SSkillItem& r)->bool{
      return r.dwID == oldskillid;
    });
    if (s == v.vecSkillItem.end())
      continue;
    s->dwReplaceID = newskillid;

    if (m_pFighter && m_pFighter->getUser() && m_pFighter->getUser()->getCurFighter())
      m_pFighter->getUser()->getCurFighter()->getSkill().addupdate(v.eProfession, s->dwID, s->dwSourceid);

    for (auto m = m_mapRecordReplaceID.begin(); m != m_mapRecordReplaceID.end(); )
    {
      if (m->second == oldskillid)
      {
        m = m_mapRecordReplaceID.erase(m);
        continue;
      }
      ++m;
    }

    m_mapRecordReplaceID[newskillid] = oldskillid;
    return true;
  }

  return false;
}

bool FighterSkill::recoverSkill(DWORD oldskillid)
{
  if (m_pFighter && m_pFighter->getProfession() != EPROFESSION_NOVICE && m_pFighter->getUser())
  {
    SceneFighter* pNoviceFighter = m_pFighter->getUser()->getFighter(EPROFESSION_NOVICE);
    if (pNoviceFighter == nullptr)
      return false;
    if (pNoviceFighter->getSkill().recoverSkill(oldskillid))
    {
      addupdate(EPROFESSION_NOVICE, oldskillid, 0);
      return true;
    }
  }

  for (auto &v : m_vecSkillData)
  {
    auto s = find_if(v.vecSkillItem.begin(), v.vecSkillItem.end(), [&](const SSkillItem& r)->bool{
      return r.dwID == oldskillid;
    });
    if (s == v.vecSkillItem.end())
      continue;
    s->dwReplaceID = 0;

    if (m_pFighter && m_pFighter->getUser() && m_pFighter->getUser()->getCurFighter() == m_pFighter)
      addupdate(v.eProfession, s->dwID, s->dwSourceid);

    for (auto it = m_mapRecordReplaceID.begin(); it != m_mapRecordReplaceID.end(); )
    {
      if (it->second == oldskillid)
      {
        it = m_mapRecordReplaceID.erase(it);
        continue;
      }
      ++it;
    }
    return true;
  }

  return false;
}

DWORD FighterSkill::getOrinalSkill(DWORD newskillid) const
{
  if (m_pFighter && m_pFighter->getProfession() != EPROFESSION_NOVICE && m_pFighter->getUser())
  {
    SceneFighter* pNoviceFighter = m_pFighter->getUser()->getFighter(EPROFESSION_NOVICE);
    if (pNoviceFighter == nullptr)
      return 0;
    DWORD oriid = pNoviceFighter->getSkill().getOrinalSkill(newskillid);
    if (oriid)
      return oriid;
  }

  auto it = m_mapRecordReplaceID.find(newskillid);
  return it != m_mapRecordReplaceID.end() ? it->second : 0;
}

bool FighterSkill::loadAcc(const BlobShareSkill& data)
{
  if (m_pFighter == nullptr || m_pFighter->getProfession() != EPROFESSION_NOVICE)
    return false;

  SSkillData* pData = getSkillData(EPROFESSION_NOVICE);
  if (pData == nullptr)
    return false;
  for (int i = 0; i < data.items_size(); ++i)
  {
    SSkillItem rItem;
    rItem.fromData(data.items(i), m_pFighter);
    pData->vecSkillItem.push_back(rItem);

    SceneUser* pUser = m_pFighter->getUser();
    if (pUser)
    {
      pUser->getPackage().checkResetRestoreBookItem(rItem.dwID);
    }
  }
  return true;
}

bool FighterSkill::saveAcc(BlobShareSkill* pData)
{
  if (pData == nullptr)
    return false;
  if (m_pFighter == nullptr || m_pFighter->getProfession() != EPROFESSION_NOVICE)
    return false;

  SSkillData* pSkillData = getSkillData(EPROFESSION_NOVICE);
  if (pSkillData == nullptr)
    return false;
  for (auto &v : pSkillData->vecSkillItem)
  {
    const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(v.dwID);
    if (pSkill == nullptr || pSkill->isShareSkill() == false)
      continue;

    v.toData(pData->add_items(), m_pFighter);
  }

  return true;
}

void FighterSkill::onDeleteChar(QWORD charid)
{
  if (m_pFighter == nullptr || m_pFighter->getProfession() != EPROFESSION_NOVICE)
    return;

  SSkillData* pSkillData = getSkillData(EPROFESSION_NOVICE);
  if (pSkillData == nullptr)
    return;
  TVecSkillItem delitems;
  for (auto &v : pSkillData->vecSkillItem)
  {
    const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(v.dwID);
    if (pSkill == nullptr || pSkill->isShareSkill() == false)
      continue;
    if (charid == v.qwOwnerID)
      delitems.push_back(v);
  }

  for (auto &v : delitems)
    removeSkill(v.dwID, v.dwSourceid, v.eSource, true);
}

bool FighterSkill::checkInBranch(EProfession profession)
{
  if(!m_pFighter)
    return false;

  if(!m_pFighter->getBranch())
    return true;

  const SBranchCFG* pCfg = RoleConfig::getMe().getBranchCFG(m_pFighter->getBranch());
  if(!pCfg)
  {
    return false;
  }

  if(pCfg->baseId == profession)
    return true;

  return pCfg->hasProfession(profession);
}

void FighterSkill::initShortcut()
{
  TVecPosSkill& vecNormal = m_mapShortcut[ESKILLSHORTCUT_NORMAL];
  if (vecNormal.size() != SKILL_MAX_CUTS + 1)
    vecNormal.resize(SKILL_MAX_CUTS + 1);
  TVecPosSkill& vecAuto = m_mapShortcut[ESKILLSHORTCUT_AUTO];
  if (vecAuto.size() != MONTH_CARD_SKILL_POS + 1)
    vecAuto.resize(MONTH_CARD_SKILL_POS + 1);

  for (auto &v : VEC_SHORTCUT_EXTEND)
  {
    TVecPosSkill& vecExtend = m_mapShortcut[v];
    if (vecExtend.size() != SKILL_MAX_CUTS + 1)
      vecExtend.resize(SKILL_MAX_CUTS + 1);
  }

  for (auto &m : m_mapShortcut)
  {
    TVecPosSkill& vecPos = m.second;
    for (size_t i = 0; i < vecPos.size(); ++i)
      vecPos[i].dwPos = i;
  }
}

// 4转(策划称为3转)技能必须使用掉150点技能点(每一转40+巅峰等级技能点30)才可以升级, 若玩家没有满足该条件升级了4转技能, 则重置所有技能
// 该函数由lua补丁调用, 存档功能也会调用
void FighterSkill::fixEvo4Skill()
{
  SceneUser* user = m_pFighter->getUser();
  if (user == nullptr || user->getEvo() < 4)
    return;

  DWORD usedpoint = 0;
  bool needcheck = false;
  for (auto v = m_vecSkillData.begin(); v != m_vecSkillData.end(); ++v)
  {
    DWORD evo = v->eProfession == EPROFESSION_NOVICE ? 0 : DWORD(v->eProfession) - DWORD(v->eProfession) / 10 * 10;
    if (evo > 4)
      continue;
    else if (evo == 4)
    {
      for (auto& skill : v->vecSkillItem)
      {
        if (skill.bLearn)
        {
          needcheck = true;
          break;
        }
      }
    }
    else
      usedpoint += v->dwUsedPoint;
  }

  if (needcheck == false)
    return;

  if (usedpoint >= MiscConfig::getMe().getPeakLevelCFG().getNeedPreSkillPointByEvo(4))
    return;

  resetSkill();
  XLOG << "[技能-3转技能异常补丁]" << user->accid << user->id << user->name << "玩家技能异常,重置所有技能" << XEND;
}

SPosData* FighterSkill::getShortcut(ESkillShortcut eType, DWORD dwPos)
{
  auto m = m_mapShortcut.find(eType);
  if (m == m_mapShortcut.end())
    return nullptr;
  if (dwPos >= m->second.size())
    return nullptr;
  return &m->second[dwPos];
}

SPosData* FighterSkill::getShortcut(ESkillShortcut eType, DWORD dwSkillID, DWORD dwSourceID)
{
  auto m = m_mapShortcut.find(eType);
  if (m == m_mapShortcut.end())
    return nullptr;
  auto v = find_if(m->second.begin(), m->second.end(), [&](const SPosData& rData) -> bool{
    return rData.dwID == dwSkillID && rData.dwSourceid == dwSourceID;
  });
  if (v == m->second.end())
    return nullptr;
  return &(*v);
}

bool FighterSkill::removeShortcut(ESkillShortcut eType, DWORD dwSkillID, DWORD dwSourceID)
{
  if (eType == ESKILLSHORTCUT_MIN)
  {
    for (DWORD d = ESKILLSHORTCUT_MIN + 1; d < ESKILLSHORTCUT_MAX; ++d)
    {
      if (d == ESKILLSHORTCUT_BEINGAUTO)
        continue;
      SPosData* pData = getShortcut(static_cast<ESkillShortcut>(d), dwSkillID, dwSourceID);
      if (pData == nullptr)
        continue;
      addPosUpdate(pData->dwID, pData->dwSourceid);
      pData->clear();
    }
    return true;
  }

  SPosData* pData = getShortcut(eType, dwSkillID, dwSourceID);
  if (pData == nullptr)
    return false;
  addPosUpdate(pData->dwID, pData->dwSourceid);
  pData->clear();
  return true;
}

void FighterSkill::setLastConcertSkillID(DWORD id)
{
  if (!id || m_dwLastConcertSkillID == id)
    return;
  if (m_dwLastConcertSkillID && m_mapSpecSkillInfo.find(m_dwLastConcertSkillID) != m_mapSpecSkillInfo.end())
    markSpecSkill(m_dwLastConcertSkillID);
  m_dwLastConcertSkillID = id;
  if (m_mapSpecSkillInfo.find(id) == m_mapSpecSkillInfo.end() && m_pFighter->getUser() && m_pFighter->getUser()->m_oBuff.hasConcertAffactSkillBuff())
    markSpecSkill(m_dwLastConcertSkillID);
}

bool FighterSkill::isSkillFamilyEnable(DWORD familyid)
{
  for (auto v = m_vecSkillData.begin(); v != m_vecSkillData.end(); ++v)
    if ((*v).isSkillFamilyEnable(familyid))
      return true;
  return false;
}
