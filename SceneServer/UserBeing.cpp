#include "UserBeing.h"
#include "SceneNpcManager.h"
#include "SceneUser.h"
#include "SkillManager.h"
#include "MiscConfig.h"

bool SBeingSkillData::fromSkillItem(const SkillItem& data)
{
  dwID = data.id();
  dwPos = data.pos();
  dwRuneSpecID = data.runespecid();
  bSelectSwitch = data.selectswitch();
  bLearn = data.learn();
  bActive = data.active();
  bNotReset = data.notreset();
  return true;
}

bool SBeingSkillData::toSkillItem(SkillItem* data)
{
  if (data == nullptr)
    return false;
  data->set_id(dwID);
  data->set_pos(dwPos);
  data->set_runespecid(dwRuneSpecID);
  data->set_selectswitch(bSelectSwitch);
  data->set_learn(bLearn);
  data->set_active(bActive);
  data->set_notreset(bNotReset);
  return true;
}

const SRuneSpecCFG* SBeingSkillData::getRuneSpecCFG()
{
  if (bSelectSwitch == false || dwRuneSpecID <= 0)
    return nullptr;
  return AstrolabeConfig::getMe().getRuneSpecCFG(dwRuneSpecID);
}

bool SBeingSkillData::toClientSpecSkillInfo(BeingSkillData* data)
{
  if (data == nullptr)
    return false;

  const SRuneSpecCFG* cfg = getRuneSpecCFG();
  if (cfg && (cfg->intRange != 0 || cfg->mapBeingItemCosts.empty() == false))
  {
    SpecSkillInfo* info = data->add_specinfo();
    if (info)
    {
      info->set_id(dwID);
      info->set_changerange(cfg->intRange);
      for (auto& v : cfg->mapBeingItemCosts)
      {
        SkillCost* cost = info->add_cost();
        if (cost)
        {
          cost->set_itemid(v.first);
          cost->set_changenum(v.second);
        }
      }
    }
  }

  return true;
}

DWORD SBeingSkillData::calcUsedSkillPoint()
{
  if (bLearn == false)
    return 0;
  DWORD id = dwID / 1000 * 1000, lv = dwID % 1000, sp = 0;
  for (DWORD i = 1; i <= lv; ++i)
  {
    if (bNotReset && i == 1)
      continue;
    const BaseSkill* skill = SkillManager::getMe().getSkillCFG(id + i);
    if (skill == nullptr)
      continue;
    sp += skill->getLevelCost();
  }
  return sp;
}

bool SSceneBeingData::fromData(const UserBeingData& data)
{
  dwID = data.id();
  dwLv = data.lv();
  dwHp = data.hp();
  bLive = data.live();
  qwExp = data.exp();
  dwUsedSkillPoint = data.usedskillpoint();
  bBattle = data.battle();
  for (int i = 0; i < data.skills_size(); ++i)
    mapSkillItem[data.skills(i).id()].fromSkillItem(data.skills(i));
  for (int i = 0; i < data.buffids_size(); ++i)
    setBuffs.insert(data.buffids(i));
  if (bLive)
    oBuff.ParseFromString(data.buff());

  dwBody = data.body();
  setBodyList.clear();
  for (int i = 0; i < data.bodylist_size(); ++i)
    setBodyList.insert(data.bodylist(i));

  pCFG = BeingConfig::getMe().getBeingCFG(dwID);
  if (pCFG == nullptr)
  {
    XERR << "[生命体-加载] id:" << dwID << "找不到配置" << XEND;
    return false;
  }

  dwVersion = data.version();
  for (DWORD d = dwVersion; d < BEING_VERSION; ++d)
  {
    if (d == 0)
      patch_1();
  }
  dwVersion = BEING_VERSION;

  return true;
}

bool SSceneBeingData::toData(UserBeingData* data)
{
  if (data == nullptr)
    return false;

  data->set_id(dwID);
  data->set_lv(dwLv);
  data->set_hp(dwHp);
  data->set_live(bLive);
  data->set_exp(qwExp);
  data->set_usedskillpoint(dwUsedSkillPoint);
  data->set_battle(bBattle);
  for (auto& v : mapSkillItem)
    v.second.toSkillItem(data->add_skills());
  for (auto buff : setBuffs)
    data->add_buffids(buff);

  data->set_body(dwBody);
  for (auto &s : setBodyList)
    data->add_bodylist(s);

  if (bLive)
  {
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(qwTempID);
    if (npc && npc->getNpcType() == ENPCTYPE_BEING)
    {
      oBuff.Clear();
      npc->m_oBuff.save(&oBuff);
    }
    if (oBuff.list_size())
    {
      string str;
      if (oBuff.SerializeToString(&str))
        data->set_buff(str.c_str(), str.size());
    }
  }

  data->set_version(dwVersion);

  return true;
}

bool SSceneBeingData::reload()
{
  pCFG = BeingConfig::getMe().getBeingCFG(dwID);
  return pCFG != nullptr;
}

bool SSceneBeingData::toClientSkillData(BeingSkillData* data, bool skill)
{
  if (data == nullptr)
    return false;

  data->set_id(dwID);

  DWORD totalsp = getMaxSkillPoint();
  data->set_leftpoint(totalsp >= dwUsedSkillPoint ? totalsp - dwUsedSkillPoint : 0);
  data->set_usedpoint(dwUsedSkillPoint);

  if (skill)
  {
    for (auto& v : mapSkillItem)
    {
      v.second.toSkillItem(data->add_items());
      v.second.toClientSpecSkillInfo(data);
    }
  }

  return true;
}

bool SSceneBeingData::toClientInfoData(BeingInfo* data)
{
  if (data == nullptr)
    return false;

  data->set_guid(qwTempID);
  data->set_beingid(dwID);
  data->set_exp(qwExp);
  data->set_lv(dwLv);
  data->set_battle(bBattle);
  data->set_summon(pUser->getUserBeing().getCurBeingID() == dwID);
  data->set_live(bLive);
  data->set_body(dwBody);
  for (auto &s : setBodyList)
    data->add_bodylist(s);
  return true;
}

void SSceneBeingData::updateData()
{
  if (pUser == nullptr || obitset.any() == false)
    return;

  BeingInfoUpdate cmd;
  cmd.set_beingid(dwID);

  auto adddata = [&](EBeingDataType etype, QWORD value, const string& name = "")
  {
    BeingMemberData* pData = cmd.add_datas();
    if (pData == nullptr)
      return;
    pData->set_etype(etype);
    pData->set_value(value);
    pData->set_data(name);
  };

  for (int i = EBEINGDATA_MIN + 1; i < EBEINGDATA_MAX; ++i)
  {
    EBeingDataType s = static_cast<EBeingDataType>(i);
    if (obitset.test(s) == false)
      continue;
    switch (s)
    {
    case EBEINGDATA_LV:
      adddata(s, dwLv);
      break;
    case EBEINGDATA_EXP:
      adddata(s, qwExp);
      break;
    case EBEINGDATA_BATTLE:
      adddata(s, bBattle);
      break;
    case EBEINGDATA_GUID:
      adddata(s, qwTempID);
      break;
    case EBEINGDATA_LIVE:
      adddata(s, bLive);
      break;
    case EBEINGDATA_SUMMON:
      adddata(s, pUser->getUserBeing().getCurBeingID() == dwID);
      break;
    case EBEINGDATA_BODY:
      adddata(s, dwBody);
      break;
    case EBEINGDATA_BODYLIST:
      {
        BeingMemberData* pData = cmd.add_datas();
        if (pData == nullptr)
          return;
        pData->set_etype(s);
        for (auto &s : setBodyList)
          pData->add_values(s);
      }
      break;
    default:
      break;
    }
  }
  obitset.reset();

  if (cmd.datas_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    pUser->sendCmdToMe(send, len);

    XDBG << "[生命体-数据更新]" << pUser->accid << pUser->id << pUser->name << "收到更新" << cmd.ShortDebugString() << XEND;
  }
}

DWORD SSceneBeingData::getMaxSkillPoint()
{
  DWORD v = 0;
  if (pUser != nullptr)
    v += pUser->getUserBeing().getSkillPoint();
  const SBeingBaseLvCFG* cfg = BeingConfig::getMe().getBeingBaseLvCFG(dwLv);
  if (cfg != nullptr)
    v += cfg->dwSkillPoint;
  return v;
}

void SSceneBeingData::addBaseExp(QWORD exp, bool notify/* = true*/)
{
  qwExp += exp;
  if (exp) obitset.set(EBEINGDATA_EXP);
  XLOG << "[生命体-添加经验]" << pUser->accid << pUser->id << pUser->name << "生命体id:" << dwID << "获得经验:" << exp << qwExp << XEND;

  bool lvup = false;
  while (true)
  {
    if (dwLv >= pUser->getLevel())
      break;
    const SBeingBaseLvCFG* cfg = BeingConfig::getMe().getBeingBaseLvCFG(dwLv + 1);
    if (cfg == nullptr)
      break;
    if (qwExp < cfg->qwExp)
      break;
    DWORD prelv = dwLv;
    qwExp -= cfg->qwExp;
    dwLv += 1;
    lvup = true;
    XLOG << "[生命体-添加经验]" << pUser->accid << pUser->id << pUser->name << "生命体id:" << dwID << "升级成功" << prelv << dwLv << XEND;
  }

  if (lvup)
  {
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(qwTempID);
    if (npc)
    {
      npc->define.setLevel(dwLv);
      npc->setCollectMark(ECOLLECTTYPE_BASE);
      npc->updateAttribute();
      npc->setAttr(EATTRTYPE_HP, npc->getAttr(EATTRTYPE_MAXHP));
      npc->refreshDataAtonce();
    }

    if (notify)
    {
      BeingSkillQuery cmd;
      BeingSkillData* skdata = cmd.add_data();
      toClientSkillData(skdata, false);

      unlockSkill(skdata);

      PROTOBUF(cmd, send, len);
      pUser->sendCmdToMe(send, len);

      obitset.set(EBEINGDATA_LV);
      updateData();
    }
    else
    {
      unlockSkill(nullptr);
    }
  }
  else if (notify)
  {
    updateData();
  }
}

void SSceneBeingData::updateAttribute()
{
  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(qwTempID);
  if (npc == nullptr || npc->define.m_oVar.m_qwOwnerID != pUser->id)
    return;
  npc->setCollectMark(ECOLLECTTYPE_BASE);
  npc->updateAttribute();
  npc->refreshDataAtonce();
}

DWORD SSceneBeingData::getSkillLv(DWORD skillgroupid)
{
  for (auto& v : mapSkillItem)
  {
    if (v.second.dwID / ONE_THOUSAND == skillgroupid)
      return v.second.dwID % ONE_THOUSAND;
  }
  if (pCFG && pCFG->dwStaticSkill)
  {
    if (pCFG->dwStaticSkill / ONE_THOUSAND == skillgroupid)
      return pCFG->dwStaticSkill % ONE_THOUSAND;
  }
  return 0;
}

bool SSceneBeingData::hasSkill(DWORD skillgroupid)
{
  for (auto& v : mapSkillItem)
  {
    if (v.second.dwID / ONE_THOUSAND == skillgroupid)
      return true;
  }
  if (pCFG && pCFG->dwStaticSkill)
  {
    if (pCFG->dwStaticSkill / ONE_THOUSAND == skillgroupid)
      return true;
  }
  return false;
}

SBeingSkillData* SSceneBeingData::getSkillByGroupID(DWORD skillgroupid)
{
  auto it = find_if(mapSkillItem.begin(), mapSkillItem.end(), [&](const pair<DWORD, SBeingSkillData>& v) -> bool {
      return v.second.dwID / ONE_THOUSAND == skillgroupid;
    });
  return &it->second;
}

const SRuneSpecCFG* SSceneBeingData::getRuneSpecCFG(DWORD skillid)
{
  auto it = mapSkillItem.find(skillid);
  if (it == mapSkillItem.end())
    return nullptr;
  return it->second.getRuneSpecCFG();
}

void SSceneBeingData::unlockSkill(BeingSkillData* data)
{
  if (pCFG == nullptr)
    return;

  for (auto& v : pCFG->mapSkillID2UnlockLv)
  {
    // 根据等级解锁技能
    if (hasSkill(v.first / ONE_THOUSAND) == false)
    {
      mapSkillItem[v.first].dwID = v.first;
      mapSkillItem[v.first].bLearn = v.second == 0; // 解锁等级为0表示该技能默认就会
      mapSkillItem[v.first].bActive = dwLv >= v.second;
      if (data)
      {
        SkillItem* item = data->add_items();
        item->set_id(v.first);
        item->set_learn(mapSkillItem[v.first].bLearn);
      }
    }
    else
    {
      SBeingSkillData* p = getSkillByGroupID(v.first / ONE_THOUSAND);
      if (p)
      {
        p->bActive = dwLv >= v.second;
        if (data)
        {
          p->toSkillItem(data->add_items());
        }
      }
    }
  }
}

bool SSceneBeingData::hasEquipedSkill()
{
  for (auto& v : mapSkillItem)
    if (v.second.bLearn && v.second.dwPos)
      return true;
  return false;
}

bool SSceneBeingData::hasNormalSkillEquiped()
{
  for (auto& v : mapSkillItem)
  {
    auto it = pCFG->mapSkillID2UnlockLv.find(v.second.dwID);
    if (it != pCFG->mapSkillID2UnlockLv.end() && it->second <= 0)
      return v.second.dwPos != 0;
  }
  return false;
}

void SSceneBeingData::addBuff(TSetDWORD ids)
{
  for (auto id : ids)
    setBuffs.insert(id);
}

bool SSceneBeingData::onRuneReset(BeingSkillData* data, DWORD specid)
{
  bool changed = false;
  for (auto& v : mapSkillItem)
  {
    if (v.second.dwRuneSpecID != specid)
      continue;
    v.second.dwRuneSpecID = 0;
    changed = true;
  }
  if (data)
    toClientSkillData(data, true);
  return changed;
}

void SSceneBeingData::delBuff(TSetDWORD ids)
{
  for (auto id : ids)
    setBuffs.erase(id);
}

bool SSceneBeingData::isRuneSpecSelected(DWORD specid)
{
  for (auto& v : mapSkillItem)
  {
    if (v.second.dwRuneSpecID == specid && v.second.bSelectSwitch)
      return true;
  }
  return false;
}

void SSceneBeingData::patch_1()
{
  if (pUser == nullptr)
  {
    XERR << "[生命体-补丁1] 主人未找到" << XEND;
    return;
  }
  map<DWORD, DWORD> mapBodyMonster;
  mapBodyMonster[1281] = 600010;
  mapBodyMonster[1282] = 600011;
  mapBodyMonster[1277] = 600020;
  mapBodyMonster[1278] = 600021;
  mapBodyMonster[1283] = 600030;
  mapBodyMonster[1284] = 600031;

  auto m = mapBodyMonster.find(dwBody);
  if (m != mapBodyMonster.end())
  {
    XLOG << "[生命体-补丁1]" << pUser->accid << pUser->id << pUser->name << "当前原bodyid :" << dwBody << "替换为monsterid :" << m->second << XEND;
    dwBody = m->second;
  }

  for (auto &m : mapBodyMonster)
  {
    auto s = setBodyList.find(m.first);
    if (s != setBodyList.end())
    {
      XLOG << "[生命体-补丁1]" << pUser->accid << pUser->id << pUser->name << "原列表bodyid :" << dwBody << "替换为monsterid :" << m.second << XEND;
      setBodyList.erase(s);
      setBodyList.insert(m.second);
    }
  }

  XLOG << "[生命体-补丁1]" << pUser->accid << pUser->id << pUser->name << "已执行" << XEND;
}

DWORD SSceneBeingData::calcUsedSkillPoint()
{
  DWORD sp = 0;
  for (auto& v : mapSkillItem)
    sp += v.second.calcUsedSkillPoint();
  return sp;
}

UserBeing::UserBeing(SceneUser* user) : m_pUser(user)
{
}

UserBeing::~UserBeing()
{
}

bool UserBeing::load(const BlobUserBeing& data)
{
  m_mapID2Being.clear();
  for (int i = 0; i < data.data_size(); ++i)
  {
    SSceneBeingData b;
    b.pUser = m_pUser;
    if (b.fromData(data.data(i)) == false)
      continue;
    const SBeingCFG* pCfg = BeingConfig::getMe().getBeingCFG(b.dwID);
    if (pCfg == nullptr)
    {
      XERR << "[生命体-加载]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << b.dwID << "配置未找到" << XEND;
      continue;
    }
    b.pCFG = pCfg;
    m_mapID2Being[b.dwID] = b;
  }
  m_dwCurBeingID = data.curbeingid();
  m_dwSkillPoint = data.skillpoint();

  createAllBeing();

  fixBeingData();
  fixUsedSkillPoint();
  return true;
}

bool UserBeing::save(BlobUserBeing* data)
{
  if (data == nullptr)
    return false;

  data->set_curbeingid(m_dwCurBeingID);
  data->set_skillpoint(m_dwSkillPoint);
  data->clear_data();
  for (auto& v : m_mapID2Being)
    if (v.second.toData(data->add_data()) == false)
      return false;

  return true;
}

bool UserBeing::loadProfessionData(const BlobUserBeing& data)
{
  if (m_dwCurBeingID > 0)
  {
    BeingOffCmd cmd;
    cmd.set_beingid(m_dwCurBeingID);
    handleBeingOffCmd(cmd, true);
  }

  load(data);

  if (m_dwCurBeingID > 0)
  {
    enterScene(m_dwCurBeingID);
  }

  return true;
}

bool UserBeing::isAlchemist()
{
  switch (m_pUser->getProfession())
  {
  case EPROFESSION_ALCHEMIST:
  case EPROFESSION_CREATOR:
  case EPROFESSION_GENETIC:
    return true;
  default:
    return false;
  }
  return false;
}

void UserBeing::createAllBeing()
{
  if (isAlchemist() == false)
    return;
  const map<DWORD, SBeingCFG>& cfgs = BeingConfig::getMe().getAllBeingCFG();
  for (auto& v : cfgs)
  {
    if (getBeingData(v.second.dwID) == nullptr)
    {
      if (addBeing(v.second.dwID) == false)
        XERR << "[生命体-初始构造]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << v.second.dwID << "构造失败" << XEND;
    }
  }
  sendData();
}

bool UserBeing::reload()
{
  for (auto & v : m_mapID2Being)
  {
    if (v.second.reload() == false)
      XERR << "[生命体-重加载] id:" << v.second.dwID << "重加载失败,配置未找到" << XEND;
  }
  return true;
}

void UserBeing::timer(DWORD cur)
{
  if (m_dwTempEndBattleTime && cur >= m_dwTempEndBattleTime)
  {
    m_qwMasterCurLockID = 0;
    m_dwTempEndBattleTime = 0;
  }

  SSceneBeingData* being = getCurBeingData();
  if (being == nullptr || being->bLive == false)
    return;
  if (being->dwTempEndBattleTime && cur >= being->dwTempEndBattleTime)
  {
    being->dwTempEndBattleTime = 0;
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(being->qwTempID);
    if (npc == nullptr || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
      return;
    if (npc->m_ai.getCurLockID() != 0)
      npc->m_ai.setCurLockID(0);

    updateState(being);
  }
}

SSceneBeingData* UserBeing::getBeingData(DWORD ID)
{
  auto it = m_mapID2Being.find(ID);
  if (it == m_mapID2Being.end())
    return nullptr;
  return &it->second;
}

SceneNpc* UserBeing::getCurBeingNpc()
{
  SSceneBeingData* being = getCurBeingData();
  if (being == nullptr)
    return nullptr;
  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(being->qwTempID);
  if (npc == nullptr || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
    return nullptr;
  return npc;
}

void UserBeing::sendData()
{
  sendBeingInfo();
  sendSkillData();
}

void UserBeing::addBaseExp(QWORD exp)
{
  exp /= 3;
  if (exp <= 0)
    return;
  SSceneBeingData* p = getCurBeingData();
  if (p == nullptr || p->bLive == false)
    return;
  DWORD oldlv = p->dwLv;
  p->addBaseExp(exp);
  if (oldlv != p->dwLv)
  {
    m_pUser->getQuest().onBeingLvUp();
  }
}

bool UserBeing::addBeing(DWORD id)
{
  if (getBeingData(id) != nullptr)
    return true;

  const SBeingCFG* pCfg = BeingConfig::getMe().getBeingCFG(id);
  if (pCfg == nullptr)
  {
    XERR << "[生命体-添加]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << id << "配置未找到" << XEND;
    return false;
  }

  SSceneBeingData data;
  data.pUser = m_pUser;
  data.dwID = id;
  data.bLive = true;
  data.pCFG = pCfg;
  data.addBaseExp(0, false);
  data.unlockSkill(nullptr);

  // 默认装备上普攻技能
  for (auto& v : data.mapSkillItem)
    if (v.second.bLearn)
      v.second.dwPos = 1;

  m_mapID2Being[id] = data;

  XLOG << "[生命体-添加]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << id << "添加成功" << XEND;
  return true;
}

bool UserBeing::summon(DWORD id)
{
  if (m_dwCurBeingID != 0)
    return false;

  if (enterScene(id) == false)
    return false;
  XLOG << "[生命体-召唤]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << id << "召唤成功" << XEND;
  return true;
}

bool UserBeing::revive(DWORD hppercent)
{
  SSceneBeingData* being = getCurBeingData();
  if (being == nullptr || being->bLive)
    return false;

  being->bLive = true;
  being->obitset.set(EBEINGDATA_LIVE);
  if (enterScene(m_dwCurBeingID, hppercent) == false)
  {
    being->bLive = false;
    being->obitset.reset(EBEINGDATA_LIVE);
    return false;
  }

  XLOG << "[生命体-复活]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << being->dwID << hppercent << "复活成功" << XEND;
  return true;
}

// 给生命体添加buff
bool UserBeing::addBufftoBeing(DWORD beingid, const TSetDWORD& buffids, xSceneEntryDynamic* fromEntry/* = nullptr*/, DWORD lv/* = 0*/)
{
  SSceneBeingData* being = getBeingData(beingid);
  if (being == nullptr || being->bLive == false)
    return false;

  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(being->qwTempID);
  if (npc == nullptr || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
    return false;

  for (auto id : buffids)
    npc->m_oBuff.add(id, fromEntry, lv);

  return true;
}

void UserBeing::changeState(SSceneBeingData* being, EBeingState state)
{
  if (being == nullptr || being->eState == state)
    return;

  switch (being->eState)
  {
  case EBEINGSTATE_NORMAL:
  case EBEINGSTATE_WAIT_BATTLE:
    break;
  default:
    break;
  }

  switch (state)
  {
  case EBEINGSTATE_NORMAL:
  case EBEINGSTATE_WAIT_BATTLE:
    break;
  default:
    return;
  }

  being->eState = state;
}

void UserBeing::updateState(SSceneBeingData* being)
{
  if (being == nullptr || being->bLive == false)
    return;

  switch (being->eState)
  {
  case EBEINGSTATE_NORMAL:
    if (now() > being->dwLastBattleTime + MiscConfig::getMe().getBeingCFG().dwOffOwnerToBattleTime)
    {
      changeState(being, EBEINGSTATE_WAIT_BATTLE);
    }
    break;
  case EBEINGSTATE_WAIT_BATTLE:
    break;
  default:
    break;
  }
}

bool UserBeing::enterScene(DWORD id, DWORD hppercent/* = 0*/)
{
  Scene* pScene = m_pUser->getScene();
  if (pScene == nullptr)
    return false;

  if (pScene->getBaseCFG() && pScene->getBaseCFG()->noBeing())
    return true;

  SSceneBeingData* being = getBeingData(id);
  if (being == nullptr)
  {
    if (addBeing(id) == false)
      return false;
    being = getBeingData(id);
    if (being == nullptr)
      return false;
  }

  if (being->bLive == false || being->qwTempID != 0)
    return false;

  NpcDefine def;
  def.setID(being->dwID);
  def.setPos(m_pUser->getPos());
  def.setRange(MiscConfig::getMe().getBeingCFG().dwBirthRange);
  def.m_oVar.m_qwOwnerID = m_pUser->id;
  def.setLife(1);
  def.setBehaviours(def.getBehaviours() & ~BEHAVIOUR_OUT_RANGE_BACK);
  def.setTerritory(0);
  def.setName(being->pCFG->strName.c_str());
  def.setLevel(being->dwLv);
  def.setDir(m_pUser->getUserSceneData().getDir() / ONE_THOUSAND);
  if (being->dwHp)
    def.m_oVar.m_dwDefaultHp = being->dwHp;

  SceneNpc* npc = SceneNpcManager::getMe().createNpc(def, pScene);
  if (npc == nullptr)
    return false;
  BeingNpc* beingNpc = dynamic_cast<BeingNpc*>(npc);
  if (beingNpc == nullptr)
  {
    npc->setClearState();
    return false;
  }
  beingNpc->m_oBuff.load(being->oBuff);

  // 玩家切线, 宠物添加打boss、mini等切线保护buff
  if (m_pUser->isJustInViceZone())
  {
    DWORD limitbuff = MiscConfig::getMe().getSystemCFG().dwZoneBossLimitBuff;
    QWORD endtime = m_pUser->m_oBuff.getBuffDelTime(limitbuff);
    if (endtime)
      beingNpc->m_oBuff.add(limitbuff, m_pUser, 0, 0, endtime);
  }

  for (auto& v : being->mapSkillItem)
  {
    if (v.second.bLearn == false)
      continue;
    const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(v.second.dwID);
    if (pSkill && pSkill->getSkillType() == ESKILLTYPE_PASSIVE)
    {
      beingNpc->m_oBuff.addSkillBuff(v.second.dwID);
    }
  }
  for (auto id : being->setBuffs)
    beingNpc->m_oBuff.add(id, m_pUser);

  being->qwTempID = beingNpc->id;
  if (hppercent)
  {
    beingNpc->setAttr(EATTRTYPE_HP, beingNpc->getAttr(EATTRTYPE_MAXHP) * hppercent / 100.0f);
    being->dwHp = beingNpc->getAttr(EATTRTYPE_HP);
  }

  m_dwCurBeingID = id;

  being->obitset.set(EBEINGDATA_GUID);
  being->obitset.set(EBEINGDATA_SUMMON);
  being->updateData();

  m_pUser->setCollectMark(ECOLLECTTYPE_DYNAMIC_BUFF);
  m_pUser->refreshDataAtonce();

  m_pUser->m_oBuff.onBeingChange();

  return true;
}

bool UserBeing::leaveScene(DWORD id)
{
  SSceneBeingData* being = getBeingData(id);
  if (being == nullptr)
    return false;

  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(being->qwTempID);
  if (npc == nullptr || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
    return false;

  being->qwTempID = 0;
  being->oBuff.Clear();
  npc->m_oBuff.save(&(being->oBuff));

  changeState(being, EBEINGSTATE_WAIT_BATTLE);
  being->dwHp = npc->getAttr(EATTRTYPE_HP);
  if (npc->isAlive())
  {
    being->bLive = true;
    npc->removeAtonce();
  }
  else
  {
    being->bLive = false;
  }

  being->obitset.set(EBEINGDATA_GUID);
  being->obitset.set(EBEINGDATA_LIVE);
  being->updateData();

  m_pUser->setCollectMark(ECOLLECTTYPE_DYNAMIC_BUFF);
  m_pUser->refreshDataAtonce();

  m_pUser->m_oBuff.onBeingChange();

  return true;
}

void UserBeing::onUserEnterScene()
{
  if (m_dwCurBeingID == 0)
    return;
  enterScene(m_dwCurBeingID);
}

void UserBeing::onUserLeaveScene()
{
  if (m_dwCurBeingID == 0)
    return;
  leaveScene(m_dwCurBeingID);
}

void UserBeing::onUserMove(const xPos& dest)
{
  Scene* pScene = m_pUser->getScene();
  if (pScene == nullptr)
    return;
  SSceneBeingData* being = getCurBeingData();
  if (being == nullptr || being->bLive == false)
    return;
  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(being->qwTempID);
  if (npc == nullptr || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
    return;
  if (getDistance(npc->getPos(), dest) < 1.5)
    return;

  // 吃料理状态特殊处理
  std::list<xPos> list;
  if (m_pUser->getScene()->findingPath(dest, m_pUser->getPos(), list, TOOLMODE_PATHFIND_FOLLOW) == false)
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
  getPosOnCircle(dest, mydest, 70, outpos);
  if (pScene->getValidPos(outpos) == false)
    return;
  npc->m_ai.moveTo(outpos);
}

void UserBeing::onUserMoveTo(const xPos& dest)
{
  // SSceneBeingData* being = getCurBeingData();
  // if (being == nullptr || being->bLive == false)
  //   return;
  // SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(being->qwTempID);
  // if (npc == nullptr)
  //   return;
  // npc->setScenePos(dest);
}

void UserBeing::onUserGoTo(const xPos& dest)
{
  SSceneBeingData* being = getCurBeingData();
  if (being == nullptr || being->bLive == false)
    return;
  changeState(being, EBEINGSTATE_NORMAL);
  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(being->qwTempID);
  if (npc == nullptr || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
    return;
  xPos pos = dest;
  if (npc->getScene())
    npc->getScene()->getRandPos(m_pUser->getPos(), 5, pos);
  npc->m_oMove.clear();
  npc->goTo(pos);
}

void UserBeing::onUserDie()
{
  SSceneBeingData* being = getCurBeingData();
  if (being == nullptr || being->bLive == false)
    return;
  changeState(being, EBEINGSTATE_WAIT_BATTLE);
}

void UserBeing::onUserAttack(xSceneEntryDynamic* enemy)
{
  if (enemy == nullptr)
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
  m_qwMasterCurLockID = enemy->id;
  m_dwTempEndBattleTime = cur + 3;

  SSceneBeingData* being = getCurBeingData();
  if (being == nullptr || being->bLive == false || being->bBattle == false || being->hasNormalSkillEquiped() == false)
    return;

  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(being->qwTempID);
  if (npc == nullptr || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
    return;
  changeState(being, EBEINGSTATE_NORMAL);
  npc->m_ai.setCurLockID(enemy->id);
  being->dwTempEndBattleTime = cur + 3;
  being->dwLastBattleTime = cur;
}

void UserBeing::onBeingDie(SceneNpc* npc)
{
  if (npc == nullptr)
    return;
  for (auto& v : m_mapID2Being)
  {
    if (v.second.qwTempID != npc->id)
      continue;
    v.second.bLive = false;
    v.second.dwHp = 0;
    npc->m_oBuff.clear();
    leaveScene(v.second.dwID);
  }
}

void UserBeing::onUserSkillLevelUp(DWORD skillid)
{
  if (isAlchemist() && m_pUser->getUserSceneData().getSkillOptValue(ESKILLOPTION_SUMMONBEING) == 0)
  {
    // 召唤生命体技能设置默认值
    const SummonBeingSkill* p = dynamic_cast<const SummonBeingSkill*>(SkillManager::getMe().getSkillCFG(skillid));
    if (p)
      m_pUser->getUserSceneData().setSkillOpt(ESKILLOPTION_SUMMONBEING, p->getFirstBeingID());
  }

  const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(skillid);
  if (pSkill == nullptr || pSkill->getLuaParam().getTableInt("update_being") != 1)
    return;
  SSceneBeingData* being = getCurBeingData();
  if (being == nullptr)
    return;
  being->updateAttribute();
}

void UserBeing::onUserSkillReset()
{
  SSceneBeingData* being = getCurBeingData();
  if (being == nullptr)
    return;
  being->updateAttribute();

  BeingOffCmd cmd;
  cmd.set_beingid(being->dwID);
  handleBeingOffCmd(cmd, true);
}

void UserBeing::onUserAttrChange()
{
  SSceneBeingData* being = getCurBeingData();
  if (being == nullptr)
    return;
  being->updateAttribute();
}

void UserBeing::onProfesChange()
{
  createAllBeing();
}

void UserBeing::onRuneReset(DWORD specid)
{
  if (m_mapID2Being.empty())
    return;

  const SRuneSpecCFG* pRuneCFG = AstrolabeConfig::getMe().getRuneSpecCFG(specid);
  if (pRuneCFG == nullptr)
    return;

  bool ntf = false;
  if (pRuneCFG->dwBeingSkillPoint > 0)
  {
    decSkillPoint(pRuneCFG->dwBeingSkillPoint);
    ntf = true;
  }
  if (pRuneCFG->mapBeingBuff.empty() == false)
  {
    for (auto& v : pRuneCFG->mapBeingBuff)
      delBuff(v.first, v.second);
    ntf = true;
  }

  BeingSkillUpdate cmd;
  for (auto& v : m_mapID2Being)
  {
    if (v.second.onRuneReset(cmd.add_update(), specid))
      ntf = true;
  }
  if (ntf && cmd.update_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

void UserBeing::onRuneReset()
{
  if (m_mapID2Being.empty())
    return;

  //重置额外技能点
  if (m_dwSkillPoint > 0)
  {
    decSkillPoint(m_dwSkillPoint);
  }

  //重置额外BUFF
  for (auto& v : m_mapID2Being)
  {
    if (v.second.qwTempID <= 0)
    {
      v.second.setBuffs.clear();
      continue;
    }

    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(v.second.qwTempID);
    if (npc == nullptr || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
      continue;

    for (auto id : v.second.setBuffs)
    {
      if (npc->m_oBuff.haveBuff(id) == false)
        continue;
      npc->m_oBuff.del(id, m_pUser);
    }
    v.second.setBuffs.clear();
  }

  //重置额外技能
  BeingSkillUpdate cmd;
  for (auto& v : m_mapID2Being)
  {
    bool change = false;
    for (auto& skill : v.second.mapSkillItem)
    {
      if (skill.second.dwRuneSpecID > 0)
      {
        skill.second.dwRuneSpecID = 0;
        change = true;
      }
    }
    if (change)
    {
      BeingSkillData* data = cmd.add_update();
      if (data)
      {
        v.second.toClientSkillData(data, true);
      }
    }
  }

  if (cmd.update_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

void UserBeing::sendSkillData(bool skill/* = true*/)
{
  BeingSkillQuery cmd;
  for (auto& v : m_mapID2Being)
  {
    BeingSkillData* data = cmd.add_data();
    if (data)
      v.second.toClientSkillData(data, skill);
  }
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void UserBeing::addSkillPoint(DWORD value)
{
  m_dwSkillPoint += value;
  sendSkillData();
}

void UserBeing::decSkillPoint(DWORD value)
{
  if (m_dwSkillPoint >= value)
    m_dwSkillPoint -= value;
  else
    m_dwSkillPoint = 0;

  // 重置技能
  resetSkill();

  sendSkillData();
}

void UserBeing::resetSkill()
{
  BeingSkillUpdate cmd;

  for (auto& v : m_mapID2Being)
  {
    DWORD maxcnt = v.second.getMaxSkillPoint();
    if (v.second.dwUsedSkillPoint > maxcnt)
    {
      auto it = MiscConfig::getMe().getBeingCFG().mapID2SkillOrder.find(v.second.dwID);
      if (it == MiscConfig::getMe().getBeingCFG().mapID2SkillOrder.end())
      {
        XERR << "[生命体-重置技能]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << v.second.dwID << "重置失败,配置未找到" << XEND;
        continue;
      }

      BeingSkillData* update = cmd.add_update();
      BeingSkillData* del = cmd.add_del();
      if (update == nullptr || del == nullptr)
        continue;
      update->set_id(v.second.dwID);
      del->set_id(v.second.dwID);

      DWORD resetcnt = v.second.dwUsedSkillPoint - maxcnt;
      for (auto skill : it->second)
      {
        for (auto& sk : v.second.mapSkillItem)
        {
          if (sk.second.bLearn == false || (sk.second.bNotReset && sk.second.dwID % 1000 == 1))
            continue;
          DWORD id = sk.second.dwID;
          if (id / 1000 == skill)
          {
            SkillItem* p = del->add_items();
            if (p == nullptr)
              break;
            p->set_id(id);

            DWORD lv = id % 1000;
            if (lv >= resetcnt)
            {
              lv -= resetcnt;
              resetcnt = 0;
            }
            else
            {
              resetcnt -= lv;
              lv = 0;
            }

            // 进阶添加的技能保留1级
            if (sk.second.bNotReset && lv == 0)
            {
              lv = 1;
              resetcnt += 1;
            }

            if (lv > 0)
            {
              DWORD newskillid = skill * 1000 + lv;
              const BaseSkill* cfg = SkillManager::getMe().getSkillCFG(newskillid);
              if (cfg == nullptr)
              {
                XERR << "[生命体-重置技能]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << v.second.dwID << "技能id:" << newskillid << "重置失败,技能未找到" << XEND;
              }
              else
              {
                SkillItem* item = update->add_items();
                if (item != nullptr)
                {
                  sk.second.toSkillItem(item);
                  item->set_id(newskillid);
                }
              }
            }
            else if (lv == 0)
            {
              SkillItem* item = update->add_items();
              if (item != nullptr)
              {
                item->set_id(skill * 1000 + 1);
                item->set_learn(false);
                item->set_active(true);
                item->set_runespecid(0);
                item->set_selectswitch(0);
                item->set_pos(0);
              }
            }
            break;
          }
        }
        if (resetcnt <= 0)
          break;
      }

      v.second.dwUsedSkillPoint = maxcnt;
      update->set_leftpoint(0);
      update->set_usedpoint(v.second.dwUsedSkillPoint);

      SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(v.second.qwTempID);

      for (int i = 0; i < del->items_size(); ++i)
      {
        if (npc)
        {
          const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(del->items(i).id());
          if (pSkill && pSkill->getSkillType() == ESKILLTYPE_PASSIVE)
            npc->m_oBuff.delSkillBuff(del->items(i).id());
        }
        v.second.mapSkillItem.erase(del->items(i).id());
        XLOG << "[生命体-重置技能]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << v.second.dwID << "技能id:" << del->items(i).id() << "删除成功" << XEND;
      }
      for (int i = 0; i < update->items_size(); ++i)
      {
        if (npc && update->items(i).learn())
        {
          const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(update->items(i).id());
          if (pSkill && pSkill->getSkillType() == ESKILLTYPE_PASSIVE)
            npc->m_oBuff.addSkillBuff(update->items(i).id());
        }
        v.second.mapSkillItem[update->items(i).id()].dwID = update->items(i).id();
        v.second.mapSkillItem[update->items(i).id()].fromSkillItem(update->items(i));
        XLOG << "[生命体-重置技能]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << v.second.dwID << "技能id:" << update->items(i).id() << "激活成功" << XEND;
      }
    }
  }

  if (cmd.update_size() > 0 || cmd.del_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

void UserBeing::resetAllSkill(DWORD beingid)
{
  BeingSkillUpdate cmd;

  BeingSkillData* update = cmd.add_update();
  if (update == nullptr)
    return;

  BeingSkillData* del = cmd.add_del();
  if (del == nullptr)
    return;

  SSceneBeingData* being = getBeingData(beingid);
  if (being == nullptr)
    return;

  being->dwUsedSkillPoint = 0;

  DWORD maxcnt = being->getMaxSkillPoint();
  update->set_id(being->dwID);
  update->set_leftpoint(maxcnt > being->dwUsedSkillPoint ? maxcnt - being->dwUsedSkillPoint : 0);
  update->set_usedpoint(being->dwUsedSkillPoint);
  del->set_id(being->dwID);

  TSetDWORD needadd;
  for (auto it = being->mapSkillItem.begin(); it != being->mapSkillItem.end(); )
  {
    if (it->second.bNotReset == false)
    {
      SkillItem* p = del->add_items();
      if (p)
        p->set_id(it->first);
      it = being->mapSkillItem.erase(it);
      continue;
    }
    else
    {
      // 进阶添加的技能保留1级
      if (it->second.dwID % 1000 != 1)
      {
        SkillItem* p = del->add_items();
        if (p)
          p->set_id(it->first);
        it->second.dwID = it->second.dwID / 1000 * 1000 + 1;
        it->second.toSkillItem(update->add_items());
        needadd.insert(it->second.dwID);
        it = being->mapSkillItem.erase(it);
        continue;
      }
    }
    ++it;
  }

  being->unlockSkill(update);

  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(being->qwTempID);

  for (int i = 0; i < del->items_size(); ++i)
  {
    if (npc)
    {
      const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(del->items(i).id());
      if (pSkill && pSkill->getSkillType() == ESKILLTYPE_PASSIVE)
        npc->m_oBuff.delSkillBuff(del->items(i).id());
    }
  }
  for (int i = 0; i < update->items_size(); ++i)
  {
    if (npc && update->items(i).learn())
    {
      const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(update->items(i).id());
      if (pSkill && pSkill->getSkillType() == ESKILLTYPE_PASSIVE)
        npc->m_oBuff.addSkillBuff(update->items(i).id());
    }
    if (needadd.find(update->items(i).id()) != needadd.end())
    {
      being->mapSkillItem[update->items(i).id()].dwID = update->items(i).id();
      being->mapSkillItem[update->items(i).id()].fromSkillItem(update->items(i));
    }
  }

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

bool UserBeing::skillLevelUp(BeingSkillLevelUp& cmd)
{
  if (cmd.skillids_size() <= 0)
    return false;

  SSceneBeingData* being = getBeingData(cmd.beingid());
  if (being == nullptr)
    return false;

  DWORD maxsp = being->getMaxSkillPoint(), usedsp = being->dwUsedSkillPoint;
  if (usedsp >= maxsp)
    return false;

  DWORD sp = 0;
  TVecDWORD delids;
  for (int i = 0; i < cmd.skillids_size(); ++i)
  {
    const BaseSkill* cfg = nullptr;
    DWORD tarid = cmd.skillids(i), srcid = 0;

    for (auto& v : being->mapSkillItem)
    {
      if (tarid / 1000 == v.second.dwID / 1000)
      {
        srcid = v.second.dwID;
        cfg = SkillManager::getMe().getSkillCFG(srcid);
        if (cfg == nullptr)
        {
          XERR << "[生命体-技能升级]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << being->dwID << srcid << "技能找不到" << XEND;
          return false;
        }
        if (v.second.bLearn == false)
          sp += cfg->getLevelCost();
        break;
      }
    }
    if (cfg == nullptr)
    {
      XERR << "[生命体-技能升级]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << being->dwID << tarid << "技能未解锁" << XEND;
      return false;
    }

    if (srcid != tarid)
    {
      delids.push_back(srcid);
      cfg = SkillManager::getMe().getSkillCFG(cfg->getNextSkillID());
      while (cfg != nullptr)
      {
        sp += cfg->getLevelCost();
        if (cfg->id == tarid)
          break;
        cfg = SkillManager::getMe().getSkillCFG(cfg->getNextSkillID());
      }
      if (cfg == nullptr)
      {
        XERR << "[生命体-技能升级]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << being->dwID << tarid << "技能错误" << XEND;
        return false;
      }
    }
  }

  if (usedsp + sp > maxsp)
  {
    XERR << "[生命体-技能升级]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << being->dwID << usedsp << sp << maxsp << "技能点数不够" << XEND;
    return false;
  }

  BeingSkillUpdate ntf;
  BeingSkillData* update = ntf.add_update();
  BeingSkillData* del = ntf.add_del();
  if (update == nullptr || del == nullptr)
    return false;

  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(being->qwTempID);

  map<DWORD, SBeingSkillData> oldskill;
  for (int i = 0; i < cmd.skillids_size(); ++i)
  {
    SBeingSkillData* p = being->getSkillByGroupID(cmd.skillids(i) / 1000);
    if (p)
      oldskill[cmd.skillids(i) / 1000] = *p;
  }

  for (auto id : delids)
  {
    SkillItem* p = del->add_items();
    if (p)
      p->set_id(id);

    if (npc)
    {
      const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(id);
      if (pSkill && pSkill->getSkillType() == ESKILLTYPE_PASSIVE)
        npc->m_oBuff.delSkillBuff(id);
    }

    being->mapSkillItem.erase(id);
    XLOG << "[生命体-技能升级]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << being->dwID << "技能id:" << id << "删除成功" << XEND;
  }
  if (del->items_size() > 0)
    del->set_id(being->dwID);

  for (int i = 0; i < cmd.skillids_size(); ++i)
  {
    if (npc)
    {
      const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(cmd.skillids(i));
      if (pSkill && pSkill->getSkillType() == ESKILLTYPE_PASSIVE)
        npc->m_oBuff.addSkillBuff(cmd.skillids(i));
    }

    auto oldsk = oldskill.find(cmd.skillids(i) / 1000);
    if (oldsk != oldskill.end())
    {
      being->mapSkillItem[cmd.skillids(i)] = oldsk->second;
    }
    being->mapSkillItem[cmd.skillids(i)].dwID = cmd.skillids(i);
    being->mapSkillItem[cmd.skillids(i)].bLearn = true;
    being->mapSkillItem[cmd.skillids(i)].bActive = true;
    XLOG << "[生命体-技能升级]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << being->dwID << "技能id:" << cmd.skillids(i) << "添加成功" << XEND;

    being->mapSkillItem[cmd.skillids(i)].toSkillItem(update->add_items());
  }

  being->dwUsedSkillPoint += sp;
  update->set_id(being->dwID);
  update->set_leftpoint(maxsp - being->dwUsedSkillPoint);
  update->set_usedpoint(being->dwUsedSkillPoint);
  XLOG << "[生命体-技能升级]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << being->dwID << "已用技能点:" << usedsp << "变为" << being->dwUsedSkillPoint << XEND;

  PROTOBUF(ntf, send, len);
  m_pUser->sendCmdToMe(send, len);

  return true;
}

void UserBeing::sendBeingInfo()
{
  BeingInfoQuery cmd;

  for (auto& v : m_mapID2Being)
  {
    v.second.toClientInfoData(cmd.add_beinginfo());
  }

  if (cmd.beinginfo_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

bool UserBeing::handleBeingSwitchState(BeingSwitchState& cmd)
{
  SSceneBeingData* being = getBeingData(cmd.beingid());
  if (being == nullptr)
    return false;
  being->bBattle = cmd.battle();
  being->obitset.set(EBEINGDATA_BATTLE);
  being->updateData();

  if (being->bBattle == false)
  {
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(being->qwTempID);
    if (npc && npc->define.m_oVar.m_qwOwnerID == m_pUser->id)
    {
      changeState(being, EBEINGSTATE_WAIT_BATTLE);
      npc->m_ai.setCurLockID(0);
      being->dwTempEndBattleTime = 0;
    }
  }

  XLOG << "[生命体-切换状态]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << being->dwID <<  "切换战斗/待命状态" << being->bBattle << XEND;
  return true;
}

bool UserBeing::handleBeingOffCmd(BeingOffCmd& cmd, bool force/* = false*/)
{
  SSceneBeingData* being = getCurBeingData();
  if (being == nullptr || being->dwID != cmd.beingid())
    return false;
  if (!force && being->bLive == false)
    return false;

  if (leaveScene(being->dwID) == false)
  {
    XERR << "[生命体-收回]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << being->dwID << "从场景移除失败" << XEND;
    return false;
  }
  if (being->bLive == false)
  {
    if (force)
    {
      being->bLive = true;
      being->dwHp = 1;
      XLOG << "[生命体-收回]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << being->dwID << "从场景移除时阵亡,强制收回" << XEND;
    }
    else
    {
      XERR << "[生命体-收回]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << being->dwID << "从场景移除时阵亡,收回失败" << XEND;
      return false;
    }
  }

  m_dwCurBeingID = 0;
  being->obitset.set(EBEINGDATA_LIVE);
  being->obitset.set(EBEINGDATA_SUMMON);
  being->updateData();

  XLOG << "[生命体-收回]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << being->dwID <<  "收回" << being->bBattle << XEND;
  return true;
}

void UserBeing::addBuff(DWORD beingid, TSetDWORD ids)
{
  SSceneBeingData* being = getBeingData(beingid);
  if (being == nullptr)
    return;
  being->addBuff(ids);

  if (being->qwTempID <= 0)
    return;

  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(being->qwTempID);
  if (npc == nullptr || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
    return;

  for (auto id : ids)
  {
    if (npc->m_oBuff.haveBuff(id))
      continue;
    npc->m_oBuff.add(id, m_pUser);
  }
}

void UserBeing::delBuff(DWORD beingid, TSetDWORD ids)
{
  SSceneBeingData* being = getBeingData(beingid);
  if (being == nullptr)
    return;
  being->delBuff(ids);

  if (being->qwTempID <= 0)
    return;

  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(being->qwTempID);
  if (npc == nullptr || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
    return;

  for (auto id : ids)
  {
    if (npc->m_oBuff.haveBuff(id) == false)
      continue;
    npc->m_oBuff.del(id, m_pUser);
  }
}

void UserBeing::switchRune(DWORD beingid, DWORD skillid, bool isopen)
{
  SSceneBeingData* being = getBeingData(beingid);
  if (being == nullptr)
    return;

  auto it = being->mapSkillItem.find(skillid);
  if (it == being->mapSkillItem.end())
    return;
  if (it->second.bSelectSwitch == isopen)
    return;
  it->second.bSelectSwitch = isopen;

  BeingSkillUpdate ntf;
  BeingSkillData* update = ntf.add_update();
  if (update == nullptr)
    return;
  it->second.toSkillItem(update->add_items());
  it->second.toClientSpecSkillInfo(update);
  DWORD totalsp = being->getMaxSkillPoint();
  update->set_id(beingid);
  update->set_leftpoint(totalsp >= being->dwUsedSkillPoint ? totalsp - being->dwUsedSkillPoint : 0);
  update->set_usedpoint(being->dwUsedSkillPoint);
  PROTOBUF(ntf, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[生命体-符文开关]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << beingid << "技能id:" << skillid << "开关:" << isopen << XEND;
}

void UserBeing::selectRuneSpecID(DWORD beingid, DWORD familySkillID, DWORD runespecid)
{
  if (m_pUser->getAstrolabes().isEffectUnlock(runespecid) == false)
    return;

  const SRuneSpecCFG* pRuneCFG = AstrolabeConfig::getMe().getRuneSpecCFG(runespecid);
  if (pRuneCFG == nullptr || pRuneCFG->setSkillIDs.find(familySkillID) == pRuneCFG->setSkillIDs.end())
    return;

  SSceneBeingData* being = getBeingData(beingid);
  if (being == nullptr)
    return;
  auto it = find_if(being->mapSkillItem.begin(), being->mapSkillItem.end(), [&](const pair<DWORD, SBeingSkillData>& v) -> bool {
      return v.second.dwID / ONE_THOUSAND == familySkillID;
    });
  if (it == being->mapSkillItem.end() || it->second.dwRuneSpecID == runespecid)
    return;
  if (it->second.bSelectSwitch)
  {
    it->second.dwRuneSpecID = runespecid;
    XLOG << "[生命体-符文技能]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << beingid << "技能id:" << it->second.dwID << "符文:" << runespecid << XEND;

    BeingSkillUpdate ntf;
    BeingSkillData* update = ntf.add_update();
    if (update == nullptr)
      return;
    it->second.toSkillItem(update->add_items());
    it->second.toClientSpecSkillInfo(update);
    DWORD totalsp = being->getMaxSkillPoint();
    update->set_id(beingid);
    update->set_leftpoint(totalsp >= being->dwUsedSkillPoint ? totalsp - being->dwUsedSkillPoint : 0);
    update->set_usedpoint(being->dwUsedSkillPoint);
    PROTOBUF(ntf, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

void UserBeing::equipSkill(const EquipSkill& cmd)
{
  if (cmd.pos() > MiscConfig::getMe().getBeingCFG().dwAutoSkillMax)
    return;

  SSceneBeingData* being = getBeingData(cmd.beingid());
  if (being == nullptr)
    return;

  auto it = being->mapSkillItem.find(cmd.skillid());
  if (it == being->mapSkillItem.end() || it->second.dwPos == cmd.pos() || it->second.bLearn == false)
    return;

  const BaseSkill* skill = SkillManager::getMe().getSkillCFG(cmd.skillid());
  if (skill && skill->getSkillType() == ESKILLTYPE_PASSIVE)
    return;

  BeingSkillUpdate ntf;
  BeingSkillData* update = ntf.add_update();
  if (update == nullptr)
    return;

  if (it->second.dwPos != 0) // 已在自动技能栏
  {
    if (cmd.pos() != 0)
    {
      for (auto& v : being->mapSkillItem)
        if (v.second.dwPos == cmd.pos())
        {
          v.second.dwPos = it->second.dwPos;
          v.second.toSkillItem(update->add_items());
          break;
        }
    }
    it->second.dwPos = cmd.pos();
    it->second.toSkillItem(update->add_items());
  }
  else
  {
    if (cmd.pos() != 0)
    {
      for (auto& v : being->mapSkillItem)
        if (v.second.dwPos == cmd.pos())
        {
          v.second.dwPos = 0;
          v.second.toSkillItem(update->add_items());
          break;
        }
    }
    it->second.dwPos = cmd.pos();
    it->second.toSkillItem(update->add_items());
  }

  if (update->items_size() > 0)
  {
    DWORD totalsp = being->getMaxSkillPoint();
    update->set_id(being->dwID);
    update->set_leftpoint(totalsp >= being->dwUsedSkillPoint ? totalsp - being->dwUsedSkillPoint : 0);
    update->set_usedpoint(being->dwUsedSkillPoint);
    PROTOBUF(ntf, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

DWORD UserBeing::getRuneSpecSkillID(DWORD beingid, DWORD skillid)
{
  SSceneBeingData* being = getBeingData(beingid);
  if (being == nullptr)
    return 0;
  const SRuneSpecCFG* cfg = being->getRuneSpecCFG(skillid);
  if (cfg == nullptr)
    return 0;
  return cfg->dwBeingSkillID;
}

int UserBeing::getRuneSpecRange(DWORD beingid, DWORD skillid)
{
  SSceneBeingData* being = getBeingData(beingid);
  if (being == nullptr)
    return 0;
  const SRuneSpecCFG* cfg = being->getRuneSpecCFG(skillid);
  if (cfg == nullptr)
    return 0;
  return cfg->intRange;
}

const SRuneSpecCFG* UserBeing::getRuneSpecCFG(DWORD beingid, DWORD skillid)
{
  SSceneBeingData* being = getBeingData(beingid);
  if (being == nullptr)
    return nullptr;
  return being->getRuneSpecCFG(skillid);
}

bool UserBeing::isRuneSpecSelected(DWORD specid)
{
  for (auto& v : m_mapID2Being)
    if (v.second.isRuneSpecSelected(specid))
      return true;
  return false;
}

bool UserBeing::changeBody(DWORD beingid, DWORD body)
{
  SSceneBeingData* being = getBeingData(beingid);
  if (being == nullptr)
    return false;

  if (being->dwBody == body)
    return true;

  auto it = being->setBodyList.find(body);
  if (it == being->setBodyList.end())
  {
    const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(beingid);
    if (pCFG == nullptr || pCFG->dwID != body)
    {
      XERR << "[生命体-换肤], 更换非法, 玩家:" << m_pUser->name << m_pUser->id << "生命体:" << beingid << "body:" << body << XEND;
      return false;
    }
  }

  being->dwBody = body;
  being->obitset.set(EBEINGDATA_BODY);
  being->updateData();

  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(being->qwTempID);
  if (npc)
  {
    npc->setDataMark(EUSERDATATYPE_BODY);
    npc->setDataMark(EUSERDATATYPE_LEFTHAND);
    npc->setDataMark(EUSERDATATYPE_RIGHTHAND);
    npc->setDataMark(EUSERDATATYPE_BACK);
    npc->setDataMark(EUSERDATATYPE_HEAD);
    npc->setDataMark(EUSERDATATYPE_FACE);
    npc->setDataMark(EUSERDATATYPE_TAIL);
    npc->setDataMark(EUSERDATATYPE_MOUNT);
    npc->setDataMark(EUSERDATATYPE_MOUTH);
    npc->setDataMark(EUSERDATATYPE_NAME);
    npc->refreshDataAtonce();
    npc->refreshDataAtonce();
  }

  XLOG << "[生命体-换肤], 更换成功, 玩家:" << m_pUser->name << m_pUser->id << "生命体:" << beingid << "新body:" << body << XEND;
  return true;
}

bool UserBeing::addBody(DWORD beingid, DWORD body)
{
  SSceneBeingData* being = getBeingData(beingid);
  if (being == nullptr)
    return false;

  auto it = being->setBodyList.find(body);
  if (it != being->setBodyList.end())
    return true;

  being->setBodyList.insert(body);
  being->obitset.set(EBEINGDATA_BODYLIST);

  being->updateData();
  m_pUser->getTip().addRedTip(EREDSYS_BEING_BODY);

  XLOG << "[生命体-获得新皮肤], 玩家:" << m_pUser->name << m_pUser->id << "生命体:" << beingid << "body:" << body << XEND;
  return true;
}

DWORD UserBeing::getBody(DWORD beingid)
{
  SSceneBeingData* being = getBeingData(beingid);
  if (being == nullptr)
    return 0;
  return being->dwBody;
}

// 如果当前没有召唤任何生命体(m_dwCurBeingID==0), 且目前拥有的生命体有死亡的(bLive==false), 则必须保证m_dwCurBeingID等于死亡的生命体
// 如果当前召唤了生命体(m_dwCurBeingID!=0), 且除去该生命体其他的生命体中有死亡的(bLive==false), 则强制复活其他生命体
void UserBeing::fixBeingData()
{
  for (auto& v : m_mapID2Being)
  {
    if (v.first != m_dwCurBeingID && v.second.bLive == false)
    {
      if (m_dwCurBeingID)
      {
        v.second.bLive = true;
        v.second.dwHp = 1;
        XLOG << "[生命体-数据异常修复]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << v.first << "强制复活生命体" << XEND;
      }
      else
      {
        m_dwCurBeingID = v.first;
        XLOG << "[生命体-数据异常修复]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体id:" << v.first << "修复变量当前生命体" << XEND;
      }
    }
  }
}

const string& UserBeing::getName(DWORD beingid)
{
  SSceneBeingData* being = getBeingData(beingid);
  if (being == nullptr || being->dwBody <= 0)
    return STRING_EMPTY;
  const SNpcCFG* npc = NpcConfig::getMe().getNpcCFG(being->dwBody);
  return npc ? npc->strName : STRING_EMPTY;
}

bool UserBeing::addSkill(DWORD beingid, DWORD skillid)
{
  SSceneBeingData* being = getBeingData(beingid);
  if (being == nullptr)
    return false;
  const BaseSkill* skill = SkillManager::getMe().getSkillCFG(skillid);
  if (skill == nullptr)
    return false;

  if (being->mapSkillItem.find(skillid) != being->mapSkillItem.end())
    return false;

  SBeingSkillData& skdata = being->mapSkillItem[skillid];
  skdata.dwID = skillid;
  skdata.bLearn = true;
  skdata.bActive = true;
  skdata.bNotReset = true;

  if (skill->getSkillType() == ESKILLTYPE_PASSIVE && being->qwTempID)
  {
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(being->qwTempID);
    if (npc && npc->define.m_oVar.m_qwOwnerID == m_pUser->id)
      npc->m_oBuff.addSkillBuff(skillid);
  }

  BeingSkillUpdate ntf;
  BeingSkillData* update = ntf.add_update();
  if (update == nullptr)
    return false;
  being->toClientSkillData(update, false);
  skdata.toSkillItem(update->add_items());

  PROTOBUF(ntf, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[生命体-新增技能]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体:" << beingid << "技能:" << skillid << XEND;
  return true;
}

bool UserBeing::checkHasSkill(DWORD beingid, DWORD skillid)
{
  SSceneBeingData* being = getBeingData(beingid);
  if (being == nullptr)
    return false;
  const BaseSkill* skill = SkillManager::getMe().getSkillCFG(skillid);
  if (skill == nullptr)
    return false;

  if (being->mapSkillItem.find(skillid) == being->mapSkillItem.end())
    return false;
  return true;
}

void UserBeing::fixUsedSkillPoint()
{
  bool reset = false;
  for (auto& v : m_mapID2Being)
  {
    DWORD usedsp = v.second.calcUsedSkillPoint();
    if (v.second.dwUsedSkillPoint != usedsp)
    {
      XLOG << "[生命体-修复已消耗技能点]" << m_pUser->accid << m_pUser->id << m_pUser->name << "生命体:" << v.first << "拥有技能点数:" << v.second.getMaxSkillPoint() << "原已消耗技能点数:" << v.second.dwUsedSkillPoint << "实际已消耗技能点数:" << usedsp << XEND;
      v.second.dwUsedSkillPoint = usedsp;
      reset = true;
    }
  }
  if (reset == false)
    return;
  resetSkill();
}

bool UserBeing::checkHasSameSkill(DWORD beingid)
{
  SSceneBeingData* being = getBeingData(beingid);
  if (being == nullptr)
    return false;
  TSetDWORD setSkillIds;
  for (auto& it : being->mapSkillItem)
  {
    DWORD skillid = it.first/1000;
    if (setSkillIds.count(skillid) > 0)
      return true;
    setSkillIds.insert(skillid);
  }
  return false;
}
