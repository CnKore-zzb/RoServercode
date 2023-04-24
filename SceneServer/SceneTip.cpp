#include "SceneTip.h"
#include "Package.h"
#include "SceneUser.h"
#include "UserConfig.h"
#include "ProtoCommon.pb.h"
#include "RecordCmd.pb.h"
#include "TipConfig.h"
#include "Menu.h"

SceneGameTip::SceneGameTip(SceneUser *pUser) : m_pUser(pUser), m_bitMask(0)
{

}

SceneGameTip::~SceneGameTip()
{

}

void SceneGameTip::initDefaultTip()
{
  for (auto v = m_mapRedTip.begin(); v != m_mapRedTip.end(); ++v)
  {
    m_bitMask.set(v->first);
    if (EREDSYS_ADD_POINT==v->first || EREDSYS_SKILL_POINT==v->first)
    {
      if (v->second.size() > 1 && v->second.at(1) == static_cast<DWORD>(ETIPFLAG_SENDED))
        v->second.at(1) = static_cast<DWORD>(ETIPFLAG_UNSEND);
    }
  }
}

void SceneGameTip::onPortrait(DWORD dwID)
{
  const SPortraitCFG* pCFG = PortraitConfig::getMe().getPortraitCFG(dwID);
    if (pCFG == nullptr)
      return;

  if (EPORTRAITTYPE_USERPORTRAIT == pCFG->eType)
    m_pUser->getTip().addRedTip(EREDSYS_ROLE_IMG);
  else if (EPORTRAITTYPE_PETPORTRAIT == pCFG->eType)
    m_pUser->getTip().addRedTip(EREDSYS_MONSTER_IMG);
  else if(EPORTRAITTYPE_FRAME == pCFG->eType)
    m_pUser->getTip().addRedTip(EREDSYS_PHOTOFRAME);
}

void SceneGameTip::onRolePoint()
{
  if (m_pUser == nullptr)
    return;

  SceneFighter *pFighter = m_pUser->getFighter();
  if (nullptr == pFighter)
    return;

  DWORD dwMinPoint = 9999;
  for (int i = EATTRTYPE_STR; i <= EATTRTYPE_LUK; ++i)
  {
    const PAttr2Point* pCFG = AttributePointConfig::getMe().getAttrPointCFG(pFighter->getAttrPoint(static_cast<EAttrType>(i)) + 1);
    if (pCFG == nullptr)
      continue;
    if (pCFG->second < dwMinPoint)
      dwMinPoint = pCFG->second;
  }
  if (pFighter->getTotalPoint() >= dwMinPoint)
    addRedTip(EREDSYS_ADD_POINT);
  else
    removeRedTip(EREDSYS_ADD_POINT);
}

void SceneGameTip::onSkillPoint()
{
  if (m_pUser == nullptr)
    return;
  SceneFighter *pFighter = m_pUser->getFighter();
  if (nullptr == pFighter)
    return;
  if (pFighter->getSkill().getSkillPoint() > 0)
    addRedTip(EREDSYS_SKILL_POINT);
  else
    removeRedTip(EREDSYS_SKILL_POINT);
}

void SceneGameTip::timer(DWORD curTime)
{
  update(curTime);
}

bool SceneGameTip::browseTip(ERedSys eRedSys, QWORD tipid/* = 0*/)
{
  if(eRedSys == EREDSYS_GUILD_APPLY || eRedSys == EREDSYS_GUILD_ICON /*|| eRedSys == EREDSYS_GUILD_CHALLENGE_ADD*/ || eRedSys == EREDSYS_GUILD_CHALLENGE_REWARD)
  {
    SyncRedTipSocialCmd cmd;
    //cmd.set_dwid(m_pUser->getGuild().id());
    cmd.set_dwid(0);
    cmd.set_charid(m_pUser->id);
    cmd.set_red(eRedSys);
    cmd.set_add(false);

    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
  }
  return removeRedTip(eRedSys, tipid);
}

bool SceneGameTip::addRedTip(ERedSys eRedSys, QWORD qwSubTip /*= 0*/)
{
  if (eRedSys <= EREDSYS_MIN || eRedSys >= EREDSYS_MAX)
    return false;

  auto m = m_mapRedTip.find(eRedSys);
  if (m != m_mapRedTip.end())
  {
    if (qwSubTip != 0)
    {
      auto v = find(m->second.begin(), m->second.end(), qwSubTip);
      if (v != m->second.end())
        return false;
      m->second.push_back(qwSubTip);
      m_bitMask.set(eRedSys);

      XLOG << "[红点-添加子]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加 type :" << eRedSys << "子节点 :" << qwSubTip << XEND;
      return true;
    }
    return false;
  }

  TVecQWORD& vecSubTip = m_mapRedTip[eRedSys];
  if (qwSubTip != 0)
    vecSubTip.push_back(qwSubTip);

  m_bitMask.set(eRedSys);
  XLOG << "[红点-添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加 type :" << eRedSys << "子节点 :" << qwSubTip << XEND;
  return true;
}

bool SceneGameTip::removeRedTip(ERedSys eRedSys, QWORD tipid/* = 0*/)
{
  if (eRedSys == EREDSYS_AUCTION_RECORD)
  {
    m_bitMask.set(eRedSys);
    return true;
  }

  auto it = m_mapRedTip.find(eRedSys);
  if (m_mapRedTip.end() == it)
    return false;

  if (tipid)
  {
    for (auto v = it->second.begin(); v != it->second.end();)
      if (*v == tipid)
      {
        v = it->second.erase(v);
        m_bitMask.set(eRedSys);
      }
      else
      {
        ++v;
      }
  }

  if (eRedSys == EREDSYS_TUTOR_TASK)
  {
    if (tipid == 0 || it->second.empty())
    {
      m_mapRedTip.erase(it);
      m_bitMask.set(eRedSys);
      XLOG << "[红点-移除]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "移除 type :" << eRedSys << XEND;
    }
  }
  else
  {
    m_mapRedTip.erase(it);
    m_bitMask.set(eRedSys);
    XLOG << "[红点-移除]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "移除 type :" << eRedSys << XEND;
  }

  return true;
}

void SceneGameTip::sendRedTip(ERedSys eRedSys)
{
  auto m = m_mapRedTip.find(eRedSys);
  if (m != m_mapRedTip.end())
    m_bitMask.set(eRedSys);
}

/*bool SceneGameTip::isTiped(ERedSys eRedSys, DWORD tipID)
{
  auto it = m_mapRedTip.find(eRedSys);
  if (m_mapRedTip.end() == it)
    return false;
  else
  {
    auto v = find(m_mapRedTip[eRedSys].begin(), m_mapRedTip[eRedSys].end(), tipID);
    if (v == m_mapRedTip[eRedSys].end())
      return false;
  }
  return true;
}

bool SceneGameTip::isTiped(ERedSys eRedSys)
{
  auto it = m_mapRedTip.find(eRedSys);
  if (it == m_mapRedTip.end())
    return false;

  return true;
}*/

bool SceneGameTip::save(Cmd::BlobTips *data)
{
  data->Clear();
  for (auto v = m_mapRedTip.begin(); v != m_mapRedTip.end(); ++v)
  {
    Cmd::BlobTipItem *pTip = data->add_list();
    if (pTip == nullptr)
    {
      XERR << "[红点-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << v->first << "create protobuf error" << XEND;
      continue;
    }

    if(v->first == EREDSYS_TEAMAPPLY || v->first == EREDSYS_TUTOR_TASK)
      continue;

    pTip->set_red(v->first);
    for (auto m = (v->second).begin(); m != (v->second).end(); ++m)
      pTip->add_tipid(*m);
  }

  XDBG << "[红点-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小 :" << data->ByteSize() << XEND;
  return true;
}

bool SceneGameTip::load(const Cmd::BlobTips &data)
{
  for (int i = 0; i < data.list_size(); ++i)
  {
    TVecQWORD vecTipID;
    Cmd::BlobTipItem rTip = data.list(i);

    // 功能屏蔽(B1915【B】【公会挑战】公会挑战点击报错，无法关闭界面)
    if (rTip.red() == EREDSYS_GUILD_CHALLENGE_ADD)
    {
      XLOG << "[红点-加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "加载红点 :" << rTip.red() << "被忽略,功能屏蔽" << XEND;
      continue;
    }

    for ( int j = 0; j < rTip.tipid_size(); ++j)
      vecTipID.push_back(rTip.tipid(j));
    m_mapRedTip.insert(make_pair(rTip.red(), vecTipID));
  }
  initDefaultTip();
  return true;
}

bool SceneGameTip::handleRedTip(const GameTipCmd &cmd)
{
  if (cmd.opt()== ETIPOPT_DELETE)
  {
     for (int i = 0; i < cmd.redtip_size(); ++i)
     {
       const RedTip &red = cmd.redtip(i);
       removeRedTip(red.redsys());
     }
  }
  else if (cmd.opt()==ETIPOPT_UPDATE)
  {
     for (int i = 0; i < cmd.redtip_size(); ++i)
     {
       const RedTip &red = cmd.redtip(i);
       //if (red.tipid_size() > 0)
       addRedTip(red.redsys());
     }
  }

  return true;
}

void SceneGameTip::update(DWORD curTime)
{
  if (m_bitMask.any() == false)
    return;

  GameTipCmd update;
  update.set_opt(ETIPOPT_UPDATE);
  GameTipCmd del;
  del.set_opt(ETIPOPT_DELETE);

  Menu& rMenu = m_pUser->getMenu();
  for (int i = EREDSYS_MIN + 1; i < EREDSYS_MAX; ++i)
  {
    if (m_bitMask.test(i) == false)
      continue;

    if (i == EREDSYS_MANUAL_MONSTER && (rMenu.isOpen(EMENUID_MANUAL_MONSTER) == false || rMenu.isOpen(EMENUID_MANUAL) == false))
      continue;
    else if (i >= EREDSYS_MANUAL_CARD_WEAPON && i <= EREDSYS_MANUAL_CARD_HEAD && (rMenu.isOpen(EMENUID_MANUAL_CARD) == false || rMenu.isOpen(EMENUID_MANUAL) == false))
      continue;
    else if (i == EREDSYS_MANUAL_NPC && (rMenu.isOpen(EMENUID_MANUAL_NPC) == false || rMenu.isOpen(EMENUID_MANUAL) == false))
      continue;
    else if (i >= EREDSYS_MANUAL_HEAD && i <= EREDSYS_MANUAL_TAIL && (rMenu.isOpen(EMENUID_MANUAL_FASHION) == false || rMenu.isOpen(EMENUID_MANUAL) == false))
      continue;
    else if (i == EREDSYS_MANUAL_MOUNT && (rMenu.isOpen(EMENUID_MANUAL_MOUNT) == false || rMenu.isOpen(EMENUID_MANUAL) == false))
      continue;
    else if (i >= EREDSYS_MANUAL_PRONTERA && i <= EREDSYS_MANUAL_GLAST && (rMenu.isOpen(EMENUID_MANUAL_SCENERY) == false || rMenu.isOpen(EMENUID_MANUAL) == false))
      continue;
    else if (i == EREDSYS_SKILL_POINT && rMenu.isOpen(EMENUID_SKILL) == false)
      continue;
    else if (i == EREDSYS_MANUAL_MONTHCARD && rMenu.isOpen(EMENUID_MANUAL) == false)
      continue;

    auto m = m_mapRedTip.find(static_cast<ERedSys>(i));
    if (m != m_mapRedTip.end())
    {
      RedTip *pRedTip = update.add_redtip();
      if (nullptr != pRedTip)
      {
        pRedTip->set_optitem(ETIPITEMOPT_ADD);
        pRedTip->set_redsys(m->first);
        for (auto n = m->second.begin(); n != m->second.end(); ++n)
          pRedTip->add_tipid(*n);
      }
    }
    else
    {
      RedTip* pTip = del.add_redtip();
      if (pTip != nullptr)
      {
        pTip->set_optitem(ETIPITEMOPT_DELETE);
        pTip->set_redsys(static_cast<ERedSys>(i));
      }
    }
  }

  if (update.redtip_size() > 0)
  {
    PROTOBUF(update, sendupdate, lenupdate);
    m_pUser->sendCmdToMe(sendupdate, lenupdate);
  }

  if (del.redtip_size() > 0)
  {
    PROTOBUF(del, senddel, lendel);
    m_pUser->sendCmdToMe(senddel, lendel);
  }

  m_bitMask.reset();
}

void SceneGameTip::patch_1()
{
  for (auto v = m_mapRedTip.begin(); v != m_mapRedTip.end();)
  {
    const STipCFG* pCFG = TipConfig::getMe().getTipConfig(v->first);
    if(pCFG != nullptr && pCFG->dwLocalRemote == 1)
    {
      ERedSys eRedSys = v->first;
      m_mapRedTip.erase(v++);
      m_bitMask.set(eRedSys);
      XLOG << "[红点-移除] 补丁: " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "移除 type :" << eRedSys << XEND;
    }
    else
      v++;
  }
}
