#include "Menu.h"
#include "SceneUser.h"
#include "SceneShop.h"

// menu
Menu::Menu(SceneUser* pUser) : m_pUser(pUser)
{

}

Menu::~Menu()
{

}

bool Menu::load(const BlobMenu &acc_data, const BlobMenu& char_data)
{
  m_setValidMenus.clear();
  for (int i = 0; i < acc_data.list_size(); ++i)
    m_setValidMenus.insert(acc_data.list(i));
  for (int i = 0; i < char_data.list_size(); ++i)
    m_setValidMenus.insert(char_data.list(i));

  return true;
}

bool Menu::save(BlobMenu *acc_data, BlobMenu* char_data)
{
  acc_data->Clear();
  char_data->Clear();
  for (auto s = m_setValidMenus.begin(); s != m_setValidMenus.end(); ++s)
  {
    const SMenuCFG* pCFG = MenuConfig::getMe().getMenuCFG(*s);
    if (pCFG == nullptr)
    {
      XERR << "[功能开启-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "id :" << *s << "未在 Table_Menu.txt 表中找到" << XEND;
      continue;
    }
    if (pCFG->acc)
      acc_data->add_list(*s);
    else
      char_data->add_list(*s);
  }

  XDBG << "[功能开启-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小 acc :" << acc_data->ByteSize() << "char :" << char_data->ByteSize() << XEND;
  return true;
}

bool Menu::isOpen(DWORD id)
{
  return m_setValidMenus.find(id) != m_setValidMenus.end();
}

void Menu::sendAllMenu()
{
  refreshNewMenu();

  MenuList cmd;
  for (auto s = m_setValidMenus.begin(); s != m_setValidMenus.end(); ++s)
    cmd.add_list(*s);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void Menu::refreshNewMenu(EMenuCond eCond /*= EMENUCOND_MIN*/)
{
  TVecDWORD vecNewMenu;
  if (eCond == EMENUCOND_MIN)
    collectValidMenu(vecNewMenu);
  else
    collectValidMenu(eCond, vecNewMenu);

  if (vecNewMenu.empty() == false)
  {
    NewMenu cmd;
    for (auto v = vecNewMenu.begin(); v != vecNewMenu.end(); ++v)
    {
      if (isOpen(*v) == true)
        continue;

      m_setValidMenus.insert(*v);
      cmd.add_list(*v);

      processMenuEvent(*v);
      m_pUser->getEvent().onMenuOpen(*v);
    }

    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
    XLOG << "[Menu-开启]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "新开启以下功能" << cmd.ShortDebugString() << XEND;
  }
}

void Menu::open(DWORD id)
{
  const SMenuCFG* pCFG = MenuConfig::getMe().getMenuCFG(id);
  if (pCFG == nullptr || isOpen(id) == true)
    return;

  m_setValidMenus.insert(id);
  processMenuEvent(id);

  NewMenu cmd;
  cmd.set_animplay(true);
  cmd.add_list(id);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[Menu-打开]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "无条件打开" << id << XEND;
}

void Menu::close(DWORD id)
{
  if (isOpen(id) == false)
    return;

  m_setValidMenus.erase(id);

  MenuList cmd;
  cmd.add_dellist(id);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[Menu-关闭]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "无条件关闭" << id << XEND;
}

void Menu::checkBackoffSkillMenu()
{
  if (m_pUser->getFighter() == nullptr)
    return;
  const TVecMenuCFG* pVecMenuCFG = MenuConfig::getMe().getCondMenuList(EMENUCOND_SKILL);
  if (pVecMenuCFG == nullptr)
    return;

  MenuList cmd;
  for (auto v = pVecMenuCFG->begin(); v != pVecMenuCFG->end(); ++v)
  {
    const SMenuCFG& rCFG = *v;
    const SMenuCondition& rCond = rCFG.stCondition;

    if (isOpen(rCFG.id) == false)
      continue;
    if (rCond.eCond != EMENUCOND_SKILL || rCond.vecParams.empty() == true)
      continue;
    bool needbackoff = false;
    DWORD affactskillid = 0;
    for (auto &d : rCond.vecParams)
    {
      if (m_pUser->getFighter()->getSkill().isSkillEnable(d) == false)
      {
        DWORD needlv = d % ONE_THOUSAND;
        if (m_pUser->getFighter()->getSkill().getSkillLv(d) >= needlv)
          continue;

        needbackoff = true;
        affactskillid = d;
        break;
      }
    }
    if (needbackoff == false)
      continue;
    auto it = m_setValidMenus.find(rCFG.id);
    if (it == m_setValidMenus.end())
      continue;
    cmd.add_dellist(rCFG.id);
    m_setValidMenus.erase(it);

    const SMenuCFG* pCFG = MenuConfig::getMe().getMenuCFG(rCFG.id);
    if (pCFG == nullptr)
      continue;

    if (pCFG->stEvent.eEvent == EMENUEVENT_SKILLGRID)
    {
      m_pUser->decSkillPos();
    }
    else if (pCFG->stEvent.eEvent == EMENUEVENT_AUTOSKILL)
    {
      m_pUser->decAutoSkillPos();
    }
    else if (pCFG->stEvent.eEvent == EMENUEVENT_EXTENDSKILL)
    {
      m_pUser->decExtendSkillPos();
    }
    else if (pCFG->stEvent.eEvent == EMENUEVENT_UNLOCKCATNUM)
    {
      m_pUser->getWeaponPet().decMaxSize();
    }
    else
    {
      continue;
    }

    XLOG << "[Menu-重置], 玩家:" << m_pUser->name << m_pUser->id << "技能ID:" << affactskillid << "menu ID:" << rCFG.id << XEND;
  }
  if (cmd.dellist_size())
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

void Menu::collectValidMenu(TVecDWORD& vecMenus)
{
  const TMapMenuCFG& mapMenuCFG = MenuConfig::getMe().getMenuList();
  for (auto m = mapMenuCFG.begin(); m != mapMenuCFG.end(); ++m)
  {
    if (m->first == EMENUID_PET_WORK_UNLOCK)
      continue;
    if (checkEnable(m->second) == true)
      vecMenus.push_back(m->first);
  }
}

void Menu::collectValidMenu(EMenuCond eCond, TVecDWORD& vecMenus)
{
  const TVecMenuCFG* pVecCFG = MenuConfig::getMe().getCondMenuList(eCond);
  if (pVecCFG == nullptr)
    return;

  for (auto &v : *pVecCFG)
  {
    if (checkEnable(v) == true)
      vecMenus.push_back(v.id);
  }
}

bool Menu::checkEnable(const SMenuCFG& rCFG)
{
  const SMenuCondition& rCond = rCFG.stCondition;

  if (isOpen(rCFG.id) == true)
    return false;

  if (rCond.eCond == EMENUCOND_OTHER)
  {
    return false;
  }
  else if (rCond.eCond == EMENUCOND_BASE_LEVEL)
  {
    if (rCond.vecParams.empty() == true || rCond.vecParams[0] > m_pUser->getLevel())
      return false;
  }
  else if (rCond.eCond == EMENUCOND_JOB_LEVEL)
  {
    if (rCond.vecParams.empty() == true || rCond.vecParams[0] > m_pUser->getJobLv())
      return false;
  }
  else if (rCond.eCond == EMENUCOND_INGUILD)
  {
    if (m_pUser->hasGuild() == false)
      return false;
  }
  else if (rCond.eCond == EMENUCOND_QUEST)
  {
    if (rCond.vecParams.empty() == true)
      return false;
    auto o = find_if(rCond.vecParams.begin(), rCond.vecParams.end(), [this](DWORD id) -> bool{
        return m_pUser->getQuest().isSubmit(id);
        });
    if (o == rCond.vecParams.end())
      return false;
  }
  else if (rCond.eCond == EMENUCOND_ACHIEVE)
  {
    if (rCond.vecParams.empty() == true)
      return false;
    auto o = find_if(rCond.vecParams.begin(), rCond.vecParams.end(), [this](DWORD id) -> bool{
        SAchieveItem* pItem = m_pUser->getAchieve().getAchieveItem(id);
        return pItem != nullptr && pItem->dwFinishTime != 0;
        });
    if (o == rCond.vecParams.end())
      return false;
  }
  else if (rCond.eCond == EMENUCOND_SKILL)
  {
    if (rCond.vecParams.empty() == true)
      return false;
    if (m_pUser->getFighter() == nullptr)
      return false;
    bool ok = true;
    for (auto &d : rCond.vecParams)
    {
      if (m_pUser->getFighter()->getSkill().isSkillEnable(d) == false)
      {
        DWORD needlv = d % ONE_THOUSAND;
        if (m_pUser->getFighter()->getSkill().getSkillLv(d) >= needlv)
          continue;

        ok = false;
        break;
      }
    }
    if (!ok) return false;
  }
  else if (rCond.eCond == EMENUCOND_PET)
  {
    if (rCond.vecParams.size() != 2)
      return false;
    SScenePetData* pPetData = m_pUser->getUserPet().getPetData(0);
    if (pPetData == nullptr)
      return false;
    if (rCond.vecParams[0] != 0 && pPetData->dwID != rCond.vecParams[0])
      return false;
    if (pPetData->dwFriendLv < rCond.vecParams[1])
      return false;
  }
  else if (rCond.eCond == EMENUCOND_TITLE)
  {
    if (rCond.vecParams.empty() == true)
      return false;
    auto s = find_if(rCond.vecParams.begin(), rCond.vecParams.end(), [&](const DWORD& rTitle) -> bool{
        return m_pUser->getTitle().hasTitle(rTitle) == true;
        });
    if (s == rCond.vecParams.end())
      return false;
  }
  else if (rCond.eCond == EMENUCOND_MANUALGROUP)
  {
    if (rCond.vecParams.empty() == true)
      return false;
    bool bSuccess = true;
    SManualItem* pItem = m_pUser->getManual().getManualItem(EMANUALTYPE_COLLECTION);
    if (pItem == nullptr)
      return false;
    for (auto &s : rCond.vecParams)
    {
      const SManualGroupCFG* pCFG = ManualConfig::getMe().getManualGroupCFG(s);
      if (pCFG == nullptr)
      {
        bSuccess = false;
        break;
      }
      for (auto &s : pCFG->setManualIDs)
      {
        SManualSubItem* pSubItem = pItem->getSubItem(s);
        if (pSubItem == nullptr || pSubItem->eStatus < EMANUALSTATUS_UNLOCK)
        {
          bSuccess = false;
          break;
        }
      }
    }
    if (!bSuccess)
      return false;
  }
  else if (rCond.eCond == EMENUCOND_MANUALUNLOCK)
  {
    if ((rCond.vecParams.size() % 3) != 0)
      return false;
    bool bSuccess = true;
    for (size_t i = 0; i < rCond.vecParams.size(); i += 3)
    {
      EManualType eType = static_cast<EManualType>(rCond.vecParams[i]);
      DWORD dwUnlockNum = 0;
      if(eType == EMANUALTYPE_FOOD)
        dwUnlockNum = m_pUser->getSceneFood().getUnlockedFood();
      else if(eType == EMANUALTYPE_ACHIEVE)
        dwUnlockNum = m_pUser->getAchieve().getUnlockedAchieveNum();
      else
        dwUnlockNum = m_pUser->getManual().getUnlockNum(eType, rCond.vecParams[i + 2]);

      if(dwUnlockNum < rCond.vecParams[i + 1])
      {
        bSuccess = false;
        break;
      }
    }
    if(!bSuccess)
      return false;
  }
  else if (rCond.eCond == EMENUCOND_COOKLV)
  {
    if (rCond.vecParams.empty() == true)
      return false;
    if (rCond.vecParams[0] > m_pUser->getSceneFood().getCookerLv())
      return false;
  }
  else if (rCond.eCond == EMENUCOND_UNLOCK)
  {
    if (rCond.vecParams.empty() == true)
      return false;

    if (rCond.vecParams[0] == ITEM_PET_WORK)
    {
      if (rCond.vecParams.size() != 4)
        return false;
      BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
      if (pMainPack == nullptr)
        return false;
      const string& guid = pMainPack->getGUIDByType(rCond.vecParams[0]);
      if (guid.empty() == true)
        return false;

      if (m_pUser->getPetAdventure().hasPet(rCond.vecParams[1], rCond.vecParams[3], rCond.vecParams[2]) == true)
        return true;
      if (m_pUser->getPackage().hasPet(rCond.vecParams[1], rCond.vecParams[3], rCond.vecParams[2]) == true)
        return true;
      if (m_pUser->getUserPet().hasPet(rCond.vecParams[1], rCond.vecParams[3], rCond.vecParams[2]) == true)
        return true;
    }

    return false;
  }
  else if (rCond.eCond == EMENUCOND_EVO)
  {
    if (rCond.vecParams.empty() == true)
      return false;

    return m_pUser->getEvo() >= rCond.vecParams[0];
  }
  else if (rCond.eCond == EMENUCOND_PROFESSION)
  {
    if (rCond.vecParams.empty() == true)
      return false;
    return m_pUser->m_oProfession.getProfessionCount() >= rCond.vecParams[0];
  }
  else if (rCond.eCond == EMENUCOND_TOWERLAYER)
  {
    SceneTower& rTower = m_pUser->getTower();
    if (rCond.vecParams[0] != 0 && rCond.vecParams[0] > rTower.getMaxLayer())
      return false;
  }
  else if (rCond.eCond == EMENUCOND_MANUALLEVEL)
  {
    if (rCond.vecParams.empty() == true)
      return false;

    Manual& rManual = m_pUser->getManual();
    if (rCond.vecParams[0] != 0 && rCond.vecParams[0] > rManual.getManualLv())
      return false;
  }
  else if (rCond.eCond == EMENUCOND_PVPCOIN)
  {
    if (rCond.vecParams.empty() == true)
      return false;

    UserSceneData& rSceneData = m_pUser->getUserSceneData();
    if (rCond.vecParams[0] != 0 && rCond.vecParams[0] > rSceneData.getPvpCoin())
      return false;
  }
  else if (rCond.eCond == EMENUCOND_WEDDING)
  {
    if (rCond.vecParams.empty() == true)
      return false;
    return m_pUser->getUserWedding().getMaritalState() == rCond.vecParams[0];
  }
  else if (rCond.eCond == EMENUCOND_TEAMPWS)
  {
    if (rCond.vecParams.size() != 2)
      return false;
    DWORD season = m_pUser->getMatchData().getPwsSeason();
    if (season != rCond.vecParams[0])
      return false;
    DWORD rank = m_pUser->getMatchData().getPwsRank();
    return rank <= rCond.vecParams[1];
  }
  else
  {
    return false;
  }

  return true;
}

void Menu::processMenuEvent(DWORD id)
{
  if (isOpen(id) == false)
    return;

  const SMenuCFG* pCFG = MenuConfig::getMe().getMenuCFG(id);
  if (pCFG == nullptr)
    return;

  // red tip
  SceneGameTip& rTip = m_pUser->getTip();
  if (id == EMENUID_SKILL)
  {
    rTip.sendRedTip(EREDSYS_SKILL_POINT);
  }
  else if (id == EMENUID_MANUAL)
  {
    rTip.sendRedTip(EREDSYS_MANUAL_MONSTER);
    rTip.sendRedTip(EREDSYS_MANUAL_MONTHCARD);
    for (DWORD d = EREDSYS_MANUAL_CARD_WEAPON; d <= EREDSYS_MANUAL_CARD_HEAD; ++d)
      rTip.sendRedTip(static_cast<ERedSys>(d));
    rTip.sendRedTip(EREDSYS_MANUAL_NPC);
    for (DWORD d = EREDSYS_MANUAL_HEAD; d <= EREDSYS_MANUAL_TAIL; ++d)
      rTip.sendRedTip(static_cast<ERedSys>(d));
    m_pUser->getAchieve().onManualOpen();
    m_pUser->getManual().sendSkillPoint();
  }
  else if (id == EMENUID_MANUAL_MONSTER)
  {
    rTip.sendRedTip(EREDSYS_MANUAL_MONSTER);
  }
  else if (id == EMENUID_MANUAL_CARD)
  {
    for (DWORD d = EREDSYS_MANUAL_CARD_WEAPON; d <= EREDSYS_MANUAL_CARD_HEAD; ++d)
      rTip.sendRedTip(static_cast<ERedSys>(d));
  }
  else if (id == EMENUID_MANUAL_NPC)
  {
    rTip.sendRedTip(EREDSYS_MANUAL_NPC);
  }
  else if (id == EMENUID_MANUAL_FASHION)
  {
    for (DWORD d = EREDSYS_MANUAL_HEAD; d <= EREDSYS_MANUAL_TAIL; ++d)
      rTip.sendRedTip(static_cast<ERedSys>(d));
  }
  else if (id == EMENUID_MANUAL_MOUNT)
  {
    rTip.sendRedTip(EREDSYS_MANUAL_MOUNT);
  }
  else if (id == EMENUID_MANUAL_SCENERY)
  {
    for (DWORD d = EREDSYS_MANUAL_PRONTERA; d <= EREDSYS_MANUAL_GLAST; ++d)
      rTip.sendRedTip(static_cast<ERedSys>(d));
  }
  else if (id == EMENUID_KANJIMOCHAO)
  {
    m_pUser->getQuest().queryOtherData(EOTHERDATA_DAILY);
  }
  else if (id == EMENUID_FOOD)
  {
    m_pUser->getSceneFood().openFunction();
  }
  else if (id == MiscConfig::getMe().getTutorCFG().dwTutorMenuID)
  {
    m_pUser->setDataMark(EUSERDATATYPE_TUTOR_ENABLE);
    m_pUser->refreshDataAtonce();
    m_pUser->saveDataNow();
    rTip.addRedTip(EREDSYS_TUTOR_TUTOR_UNLOCK);
  }
  else if (id == EMENUID_FOOD_PACK)
  {
    m_pUser->getPackage().resetFoodPackItem();
    m_pUser->getPackage().resetFoodItem();
  }
  else if (id == EMENUID_DEAD)
  {
    m_pUser->getVar().setVarValue(EVARTYPE_DEAD_QUEST, 0);
    m_pUser->getQuest().resetDeadQuest();
  }
  else if (id == EMENUID_PET_WORK_MANUAL)
  {
    m_pUser->getPackage().resetPetWorkItem();
  }
  else if (id == EMENUID_QUEST_MANUAL)
  {
    m_pUser->getPackage().resetQuestManualItem();
  }
  else if (id == EMENUID_TUTOR_STUDENT)
  {
    rTip.addRedTip(EREDSYS_TUTOR_STUDENT_UNLOCK);
  }
  else if(id == EMENUID_PEAK_LEVEL)
  {
    rTip.addRedTip(EREDSYS_PEAK_LEVEL);
  }

  // event
  if (pCFG->stEvent.eEvent == EMENUEVENT_SKILLGRID)
  {
    m_pUser->addMaxSkillPos();
  }
  else if (pCFG->stEvent.eEvent == EMENUEVENT_AUTOSKILL)
  {
    m_pUser->addAutoSkillPos();
  }
  else if (pCFG->stEvent.eEvent == EMENUEVENT_EXTENDSKILL)
  {
    m_pUser->addExtendSkillPos();
  }
  else if (pCFG->stEvent.eEvent == EMENUEVENT_UNLOCKSHOP)
  {
    if (pCFG->stEvent.vecParams.size() == MENUEVENT_UNLOCKSHOP_PARAM_COUNT && (pCFG->stEvent.vecParams[2] == EMENUSHOPEVENT_MANUAL || pCFG->stEvent.vecParams[2] == EMENUSHOPEVENT_ALL))
    {
      Manual& oManual = m_pUser->getManual();
      TVecShopItem vecItems;
      ShopConfig::getMe().collectShopItem(pCFG->stEvent.vecParams[0], pCFG->stEvent.vecParams[1], vecItems);
      for (auto v = vecItems.begin(); v != vecItems.end(); ++v)
      {
        if (v->lv() < pCFG->stEvent.vecParams[3] || v->lv() > pCFG->stEvent.vecParams[4])
          continue;

        //for (int i = 0; i < v->items_size(); ++i)
        //  oManual.onItemAdd(v->items(i).id());
        oManual.onItemAdd(v->itemid());
      }
    }
    else
    {
      XERR << "[Menu::processMenuEvent]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "menuid :" << id << "param error" << XEND;
    }
  }
  else if (pCFG->stEvent.eEvent == EMENUEVENT_UNLOCKMANUAL)
  {
    if (pCFG->stEvent.vecParams.empty() == false)
    {
      if (pCFG->stEvent.vecParams[0] == EMANUALTYPE_FASHION)
      {
        for (size_t i = 1; i < pCFG->stEvent.vecParams.size(); ++i)
          m_pUser->getManual().onItemAdd(pCFG->stEvent.vecParams[i], false);
      }
      else if (pCFG->stEvent.vecParams[0] == EMANUALTYPE_MAP)
      {
        for (size_t i = 1; i < pCFG->stEvent.vecParams.size(); ++i)
          m_pUser->getManual().onEnterMap(pCFG->stEvent.vecParams[i], false);
      }
      else if (pCFG->stEvent.vecParams[0] == EMANUALTYPE_MONSTER)
      {
        for (size_t i = 1; i < pCFG->stEvent.vecParams.size(); ++i)
          m_pUser->getManual().onKillMonster(pCFG->stEvent.vecParams[i], false);
      }
      else if(pCFG->stEvent.vecParams[0] == EMANUALTYPE_CARD)
      {
        for (size_t i = 1; i < pCFG->stEvent.vecParams.size(); ++i)
          m_pUser->getManual().onItemAdd(pCFG->stEvent.vecParams[i], false, true);
      }
      else if (pCFG->stEvent.vecParams[0] == EMANUALTYPE_PET)
      {
        for (size_t i = 1; i < pCFG->stEvent.vecParams.size(); ++i)
          m_pUser->getManual().onPetHatch(pCFG->stEvent.vecParams[i], false);
      }
    }
    else
    {
      XERR << "[Menu::processMenuEvent]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "menuid :" << id << "param error" << XEND;
    }
  }
  else if (pCFG->stEvent.eEvent == EMENUEVENT_SCENERY)
  {
    if (pCFG->stEvent.vecParams.empty() == false)
    {
      for (size_t i = 0; i < pCFG->stEvent.vecParams.size(); ++i)
      {
        m_pUser->getScenery().add(pCFG->stEvent.vecParams[i]);
      }
    }
    else
    {
      XERR << "[Menu::processMenuEvent]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "menuid :" << id << "param error" << XEND;
    }
  }
  else if (pCFG->stEvent.eEvent == EMENUEVENT_UNLOCKACTION || pCFG->stEvent.eEvent == EMENUEVENT_UNLOCKEXPRESSION)
  {/*解锁动作, 解锁表情*/  
    do
    {
      if (pCFG->stEvent.vecParams.empty())
      {
        XERR << "[Menu::processMenuEvent]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "menuid :" << id << "param error" << XEND;
        break;
      }
      for (size_t i = 0; i < pCFG->stEvent.vecParams.size(); ++i)
      {
        DWORD dwId = pCFG->stEvent.vecParams[i];
        if (pCFG->stEvent.eEvent == EMENUEVENT_UNLOCKACTION)
          m_pUser->getUserSceneData().addAction(dwId);
        else if (pCFG->stEvent.eEvent == EMENUEVENT_UNLOCKEXPRESSION)
          m_pUser->getUserSceneData().addExpression(dwId);
      }
    } while (0);
  }
  else if (pCFG->stEvent.eEvent == EMENUEVENT_UNLOCKHAIR)
  {
    for (auto &v : pCFG->stEvent.vecParams)
    {
      const SItemCFG* pItemCFG = ItemConfig::getMe().getHairCFG(v);
      if (pItemCFG == nullptr)
      {
        XERR << "[Menu::processMenuEvent]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "menuid :" << id << "hairid :" << v << "未在Table_Hair.txt表中找到" << XEND;
        continue;
      }
      m_pUser->getHairInfo().addNewHair(pItemCFG->dwTypeID);
    }
  }
  else if (pCFG->stEvent.eEvent == EMENUEVENT_UNLOCKEYE)
  {
    for (auto &v : pCFG->stEvent.vecParams)
    {
      const SItemCFG* pItemCFG = ItemConfig::getMe().getItemCFG(v);
      if (pItemCFG == nullptr)
      {
        XERR << "[Menu::processMenuEvent]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "menuid :" << id << "eyeid :" << v << "未在Table_Item.txt表中找到" << XEND;
        continue;
      }
      m_pUser->getEye().addNewEye(pItemCFG->dwTypeID);
    }
  }
  else if (pCFG->stEvent.eEvent == EMENUEVENT_SEENPC)
  {
    for (auto &v : pCFG->stEvent.vecParams)
      m_pUser->addSeeNpc(v);
  }
  else if (pCFG->stEvent.eEvent == EMENUEVENT_HIDENPC)
  {
    for (auto &v : pCFG->stEvent.vecParams)
      m_pUser->addHideNpc(v);
  }
  else if (pCFG->stEvent.eEvent == EMENUEVENT_UNLOCKCATNUM)
  {
    m_pUser->getWeaponPet().setMaxSize(m_pUser->getWeaponPet().getMaxSize() + 1);
  }
  else if (pCFG->stEvent.eEvent == EMENUEVENT_UNLOCKCATID)
  {
    for (auto &v : pCFG->stEvent.vecParams)
      m_pUser->getWeaponPet().unlock(v);
  }
  else if (pCFG->stEvent.eEvent == EMENUEVENT_ADDSHOPCNT)
  {
    if (pCFG->stEvent.vecParams.size() == 2)
    {
      DWORD shopid = pCFG->stEvent.vecParams[0];
      DWORD count = pCFG->stEvent.vecParams[1];
      m_pUser->getSceneShop().addAccBuyLimitCnt(shopid, count);
    }
    else
    {
      XERR << "[Menu::processMenuEvent], 参数错误, 玩家:" << m_pUser->name << m_pUser->id << "menuid:" << id << XEND;
    }
  }
  else if(pCFG->stEvent.eEvent == EMENUEVENT_ADDMAXJOBLEVEL)
  {
    if (pCFG->stEvent.vecParams.empty() == false)
    {
      m_pUser->getCurFighter()->setMaxJobLv(pCFG->stEvent.vecParams[0]);
    }
  }
}

