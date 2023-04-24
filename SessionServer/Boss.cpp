#include "Boss.h"
#include "SessionScene.h"
#include "BossSCmd.pb.h"
#include "xCommand.h"
#include "SessionSceneManager.h"
#include "SessionUser.h"
#include "xDBFields.h"
#include "SessionServer.h"
#include "SessionUserManager.h"
#include "GlobalManager.h"

// Boss
Boss::Boss(const SBossCFG *b):base(b)
{
  id = base->dwID;
}

Boss::~Boss()
{
}

// Mini
Mini::Mini(const SBossCFG *b, DWORD mapid) : Boss(b), m_dwMapID(mapid)
{
}

Mini::~Mini()
{
}

// BossList
BossList::BossList()
{
}

BossList::~BossList()
{
  save();

  for (auto &it : m_list)
  {
    SAFE_DELETE(it.second);
  }
  m_list.clear();

  for (auto &it : m_miniList)
  {
    SAFE_DELETE(it.second);
  }
  m_miniList.clear();
}

bool BossList::doUserCmd(SessionUser *user, const BYTE *buf, WORD len)
{
  if (!user || !buf || !len) return false;
  xCommand *cmd = (xCommand *)buf;
  // DWORD cur = now();
  switch (cmd->param)
  {
    case BOSS_LIST_USER_CMD:
      {
        Cmd::BossListUserCmd message;
        for (auto &it : m_list)
        {
          const SBossCFG* pCFG = it.second->base;
          if (pCFG == nullptr)
          {
            XERR << "[Boss-请求列表]" << user->accid << user->id << user->getProfession() << user->name << "请求列表 id :" << it.first << "未包含正确的配置,被忽略" << XEND;
            continue;
          }
          if (pCFG->eType == EBOSSTYPE_DEAD && it.second->getSetTime() == 0)
            continue;

          Cmd::BossInfoItem *item = message.add_bosslist();
          item->set_id(it.second->id);
          //if (BossState::WaitRefresh==it.second->getState())
            item->set_refreshtime(it.second->getRefreshTime());
          //   item->set_refreshstate(getRereshState(cur, it.second->m_dwRefreshTime));
          // else
          //   item->set_refreshstate(EBOSSREFRESHSTATE_APPEARED);
          item->set_dietime(it.second->getDieTime());
          item->set_summontime(it.second->m_dwSummonTime);
          item->set_charid(it.second->getLastKillID());
          item->set_lastkiller(it.second->getLastKiller());
          item->set_mapid(it.second->getMapID());
          item->set_settime(it.second->getSetTime());
          item->set_lv(it.second->getDeadLv());
        }

        const DeadBossInfo& rBossInfo = GlobalManager::getMe().getGlobalBoss();
        if (rBossInfo.charid() != 0)
        {
          BossConfig::getMe().foreach(EBOSSTYPE_DEAD, [&](const SBossCFG& r)
          {
            Boss* pDead = get(r.dwDeadID);
            if (pDead == nullptr)
              pDead = get(r.dwID);
            if (pDead == nullptr)
            {
              XERR << "[Boss-请求列表]" << user->accid << user->id << user->getProfession() << user->name << "请求列表 id :" << r.dwID << "deadid :" << r.dwDeadID << "未找到,被忽略" << XEND;
              return;
            }
            if (pDead == nullptr)
              return;
            Cmd::BossInfoItem* pItem = message.add_deadlist();
            pItem->set_id(r.dwID);
            pItem->set_lastkiller(pDead->m_strDeadLastKiller);
            pItem->set_charid(pDead->m_qwDeadLastKillerID);
            pItem->set_settime(pDead->getSetTime());
            pItem->set_dietime(pDead->getDieTime());
            pItem->set_lv(pDead->getDeadLv());
          });
        }

        if (user->hasMonthCard())
        {
          for (auto &it : m_miniList)
          {
            Cmd::BossInfoItem *item = message.add_minilist();
            if (item == nullptr) continue;
            item->set_id(it.second->id);
            if (BossState::WaitRefresh==it.second->getState())
              item->set_refreshtime(it.second->getRefreshTime());
            //   item->set_refreshstate(getRereshState(cur, it.second->m_dwRefreshTime));
            // else
            //   item->set_refreshstate(EBOSSREFRESHSTATE_APPEARED);
            item->set_lastkiller(it.second->m_strLastKiller);
            item->set_mapid(it.second->getMapID());
            item->set_charid(it.second->m_qwLastKillerID);
          }
        }
        PROTOBUF(message, send, len);
        user->sendCmdToMe(send, len);
        XDBG << "[Boss-请求列表]" << user->accid << user->id << user->getProfession() << user->name << "数据为" << message.ShortDebugString() << XEND;
        return true;
      }
      break;
    case BOSS_USER_INFO_CMD:
      {
        PARSE_CMD_PROTOBUF(QueryKillerInfoBossCmd, rev);
        auto getKiller = [&]() -> Boss*
        {
          for (auto &it : m_list)
          {
            if (it.second->m_qwLastKillerID == rev.charid())
              return it.second;
          }
          if (user->hasMonthCard())
          {
            for (auto &it : m_miniList)
            {
              if (it.second->m_qwLastKillerID == rev.charid())
                return it.second;
            }
          }
          return nullptr;
        };
        Boss* pBoss = getKiller();
        if (!pBoss)
        {
          // 已过期, 找不到玩家, 通知前端
          rev.set_charid(0);
          PROTOBUF(rev, retsend, retlen);
          user->sendCmdToMe(retsend, retlen);
          return true;
        }

        BossKillerData* pData = rev.mutable_userdata();
        if (pData == nullptr)
          return true;
        pData->set_charid(rev.charid());

        auto toData = [&](GCharReader* p)
        {
          if (p == nullptr)
            return;
          pData->set_portrait(p->getPortrait());
          pData->set_baselevel(p->getBaseLevel());
          pData->set_hair(p->getHair());
          pData->set_haircolor(p->getHairColor());
          pData->set_body(p->getBody());
          pData->set_head(p->getHead());
          pData->set_face(p->getFace());
          pData->set_mouth(p->getMouth());
          pData->set_eye(p->getEye());
          pData->set_blink(p->getBlink());
          pData->set_profession(p->getProfession());
          pData->set_gender(p->getGender());
          pData->set_name(p->getName());
          pData->set_guildname(p->getGuildName());
        };
        SessionUser* pTar = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pTar)
        {
          GCharReader* pTarGChar = pTar->getGCharData();
          if (pTarGChar)
          {
            toData(pTarGChar);
            pBoss->m_oKillerData.CopyFrom(*pData);
          }
        }
        else
        {
          if (pBoss->m_oKillerData.charid())
          {
            pData->CopyFrom(pBoss->m_oKillerData);
          }
          else
          {
            GCharReader gChar(thisServer->getRegionID(), rev.charid());
            if (gChar.getBySocial())
            {
              toData(&gChar);
              pBoss->m_oKillerData.CopyFrom(*pData);
            }
          }
        }
        PROTOBUF(rev, retsend, retlen);
        user->sendCmdToMe(retsend, retlen);
      }
      break;
    default:
      break;
  }
  return false;
}

bool BossList::loadConfig()
{
  m_oDeadSetFunc = [&]()
  {
    const DeadBossInfo& rBossInfo = GlobalManager::getMe().getGlobalBoss();
    if (rBossInfo.charid() == 0)
    {
      XDBG << "[Boss-点名] 点名失败,功能未开放" << XEND;
      return;
    }

    const SBossMiscCFG& rCFG = MiscConfig::getMe().getBossCFG();
    const string& first = m_bBossFuncFirst ? "首次" : "后续";
    EBossType eDestType = EBOSSTYPE_MIN;
    if (m_bBossFuncFirst)
    {
      eDestType = EBOSSTYPE_WORLD;
      m_bBossFuncFirst = false;
      m_bBossFuncSecond = true;
    }
    else if (m_bBossFuncSecond)
    {
      eDestType = EBOSSTYPE_DEAD;
      m_bBossFuncSecond = false;
    }
    else
    {
      DWORD dwRand = randBetween(0, 10000);
      eDestType = dwRand < rCFG.dwDeadSetRate ? EBOSSTYPE_DEAD : EBOSSTYPE_WORLD;
    }

    TSetDWORD setExcludeIDs;
    if (eDestType == EBOSSTYPE_DEAD)
    {
      for (auto &m : m_list)
      {
        if (m.second->base != nullptr && m.second->base->eType == EBOSSTYPE_DEAD)
          setExcludeIDs.insert(m.second->base->dwID);
      }
    }

    const string& boss = eDestType == EBOSSTYPE_DEAD ? "亡者boss" : "世界boss";
    const SBossCFG* pCFG = BossConfig::getMe().randSetBossCFG(eDestType, setExcludeIDs);
    if (pCFG == nullptr)
    {
      XERR << "[Boss-点名]" << first << "点名类型为" << boss << "随机boss失败,踢出列表" << setExcludeIDs << XEND;
      return;
    }

    Boss* pBoss = eDestType == EBOSSTYPE_DEAD ? get(pCFG->dwDeadID) : getWorld(pCFG->dwID);
    if (pBoss == nullptr)
    {
      XERR << "[Boss-点名]" << first << "点名类型为" << boss << "随机boss成功, cfgid :" << pCFG->dwID << "被点名, 更新boss状态失败,未找到Boss对象" << XEND;
      return;
    }

    if (eDestType == EBOSSTYPE_DEAD)
    {
      pBoss->base = pCFG;
    }
    else if (eDestType == EBOSSTYPE_WORLD)
    {
      if (pBoss->getSetTime() != 0)
      {
        XERR << "[Boss-点名]" << first << "点名类型为" << boss << "随机boss成功, cfgid :" << pCFG->dwID << "被点名, 更新boss" << pBoss->id << "状态失败,已处于点名状态" << XEND;
        return;
      }
      pBoss->m_dwMapID = pBoss->getMapID();
    }

    pBoss->setSetTime(now());
    XLOG << "[Boss-点名]" << first << "点名类型为" << boss << "随机boss成功, cfgid :" << pCFG->dwID << "被点名, 更新boss" << pBoss->id << "状态成功" << XEND;
  };

  m_oTimeWheel.clear();
  const SBossMiscCFG& rCFG = MiscConfig::getMe().getBossCFG();
  for (auto &s : rCFG.setRefreshTimes)
  {
    SWheel stWheel;
    stWheel.dwTime = s;
    stWheel.func = m_oDeadSetFunc;
    m_oTimeWheel.addWheel(stWheel);
  }

  save();

  for (auto &it : m_list)
    SAFE_DELETE(it.second);
  m_list.clear();

  for (auto &it : m_miniList)
    SAFE_DELETE(it.second);
  m_miniList.clear();

  auto createf = [&](const SBossCFG& rCFG)
  {
    if (!rCFG.blInit)
      return;

    if (rCFG.getType() == EBOSSTYPE_MVP)
    {
      Boss *boss = NEW Boss(&rCFG);
      if (!add(boss))
      {
        XERR << "[Boss-初始化] id :" << rCFG.dwID << "添加失败" << XEND;
        SAFE_DELETE(boss);
      }
    }
    else if (rCFG.getType() == EBOSSTYPE_MINI)
    {
      const TVecDWORD& maps = rCFG.getAllMap();
      for (auto &mapid : maps)
      {
        Mini *mini = NEW Mini(&rCFG, mapid);
        if (!add(mini))
        {
          XERR << "[Boss-初始化] id :" << rCFG.dwID << "添加失败" << XEND;
          SAFE_DELETE(mini);
          continue;
        }
      }
    }
    else if (rCFG.getType() == EBOSSTYPE_DEAD)
    {
      if (rCFG.dwDeadID == 0)
      {
        Boss* pBoss = NEW Boss(&rCFG);
        if (!add(pBoss))
        {
          XERR << "[Boss-初始化] id :" << rCFG.dwID << "添加失败" << XEND;
          SAFE_DELETE(pBoss);
        }
      }
    }
    else if (rCFG.getType() == EBOSSTYPE_WORLD)
    {
      Boss* world = NEW Boss(&rCFG);
      if (!addWorld(world))
      {
        XERR << "[Boss-初始化] id :" << rCFG.dwID << "添加失败" << XEND;
        SAFE_DELETE(world);
      }
    }
  };
  BossConfig::getMe().foreach(createf);

  load();

  return true;
}

bool BossList::add(Boss *boss)
{
  if (!boss || !boss->id) return false;
  m_list[boss->id] = boss;
  return true;
}

bool BossList::add(Mini *mini)
{
  if (!mini || !mini->id) return false;
  m_miniList[BossList::getMiniKey(mini->id, mini->getMapID())] = mini;
  return true;
}

bool BossList::addWorld(Boss *boss)
{
  if (!boss || !boss->id) return false;
  m_worldList[boss->id] = boss;
  return true;
}

Boss* BossList::get(DWORD id)
{
  auto it = m_list.find(id);
  if (it!=m_list.end()) return it->second;
  return NULL;
}

Boss* BossList::getWorld(DWORD id)
{
  auto it = m_worldList.find(id);
  if (it != m_worldList.end()) return it->second;
  return nullptr;
}

Mini* BossList::get(DWORD id, DWORD mapid)
{
  auto it = m_miniList.find(BossList::getMiniKey(id, mapid));
  if (it != m_miniList.end()) return it->second;
  return NULL;
}

void BossList::onSceneOpen(SessionScene *scene)
{
  if (!scene) return;

}

void BossList::onSceneClose(SessionScene *scene)
{
  if (!scene) return;

  for (auto it=m_list.begin(); it!=m_list.end(); ++it)
  {
    if (it->second->getMapID()==scene->id)
    {
      if (BossState::Refreshed==it->second->getState())
      {
        it->second->setState(BossState::WaitRefresh);
      }
    }
  }

  for (auto it=m_miniList.begin(); it!=m_miniList.end(); ++it)
  {
    if (it->second->getMapID()==scene->id)
    {
      if (BossState::Refreshed==it->second->getState())
      {
        it->second->setState(BossState::WaitRefresh);
      }
    }
  }

  for (auto it=m_worldList.begin(); it!=m_worldList.end(); ++it)
  {
    if (it->second->getMapID()==scene->id)
    {
      if (BossState::Refreshed==it->second->getState())
      {
        it->second->setState(BossState::WaitRefresh);
      }
    }
  }
}

void BossList::onBossDie(DWORD npcid, const char *killer, QWORD killerId, DWORD mapid, bool reset)
{
  if (killer == nullptr)
  {
    XERR << "[Boss-击杀] map :" << mapid << "id :" << npcid << "被" << killerId << "击杀, 更新状态失败,击杀者名字不合法" << XEND;
    return;
  }
  const SBossCFG* pBase = BossConfig::getMe().getBossCFG(npcid);
  if (pBase == nullptr)
  {
    XERR << "[Boss-击杀] map :" << mapid << "id :" << npcid << "被" << killerId << killer << "击杀, 更新状态失败,该boss未在 Table_Boss.txt 表中找到" << XEND;
    return;
  }

  DWORD curTime = now();

  if (pBase->getType() == EBOSSTYPE_MVP || pBase->getType() == EBOSSTYPE_DEAD)
  {
    Boss* pBoss = pBase->getType() == EBOSSTYPE_DEAD ? get(pBase->dwDeadID) : get(npcid);
    if (pBoss == nullptr)
    {
      XERR << "[Boss-击杀] map :" << mapid << "id :" << npcid << "被" << killerId << killer << "击杀, 更新状态失败,未找到boss实体对象" << XEND;
      return;
    }

    if (pBase->getType() == EBOSSTYPE_DEAD)
    {
      pBoss->base = BossConfig::getMe().getBossCFG(pBoss->id);
      if (pBoss->base == nullptr)
        XERR << "[Boss-击杀] map :" << mapid << "id :" << npcid << "被" << killerId << killer << "击杀, 更新状态中,从亡者boss" << npcid << "变普通boss" << pBoss->id << "失败" << XEND;
      else
        XLOG << "[Boss-击杀] map :" << mapid << "id :" << npcid << "被" << killerId << killer << "击杀, 更新状态中,从亡者boss" << npcid << "变普通boss" << pBoss->id << XEND;
    }
    if (pBoss->base == nullptr)
    {
      XERR << "[Boss-击杀] map :" << mapid << "id :" << npcid << "被" << killerId << killer << "击杀, 更新状态失败,该boss未包含正确的配置" << XEND;
      return;
    }

    pBoss->m_dwRefreshTime = reset ? curTime : curTime + calcRefreshTime(pBoss->base->getReliveTime(pBoss->getMapID()) * MIN_T);
    pBoss->setDieTime(curTime);
    pBoss->m_strLastKiller.clear();
    pBoss->setState(BossState::WaitRefresh);

    if (pBase->getType() == EBOSSTYPE_DEAD)
      pBoss->setSetTime(0);
    else if (pBase->getType() == EBOSSTYPE_MVP && pBoss->getSetTime() != 0)
      pBoss->m_dwRefreshTime = 0;

    if (killerId)
    {
      if (pBase->getType() == EBOSSTYPE_DEAD)
        pBoss->m_strDeadLastKiller = killer;
      else
        pBoss->m_strLastKiller = killer;

      if (pBase->getType() == EBOSSTYPE_DEAD)
        pBoss->m_qwDeadLastKillerID = killerId;
      else
        pBoss->m_qwLastKillerID = killerId;

      if (!reset && pBase->eType == EBOSSTYPE_DEAD)
        pBoss->setDeadLv(pBoss->getDeadLv() + 1);
    }
    else
    {
      if (!reset && pBase->eType == EBOSSTYPE_DEAD)
        pBoss->setDeadLv(pBoss->getDeadLv() > 1 ? pBoss->getDeadLv() - 1 : 1);
    }
    //pBoss->m_oKillerData.Clear();
    pBoss->setState(BossState::WaitRefresh);

    XLOG << "[Boss-击杀] map :" << mapid << "id :" << npcid << "被" << killerId << killer
      << "击杀, 更新状态成功, dietime :" << pBoss->m_dwDieTime << "refreshTime :" << pBoss->m_dwRefreshTime << "settime :" << pBoss->m_dwSetTime
      << "killerid :" << pBoss->m_qwLastKillerID << "killer :" << pBoss->m_strLastKiller << "deadkillerid :" << pBoss->m_qwDeadLastKillerID << "deadkiller :" << pBoss->m_strDeadLastKiller<< XEND;
  }
  else if (pBase->getType() == EBOSSTYPE_MINI)
  {
    Mini *pMini = get(npcid, mapid);
    if (pMini)
    {
      if (pMini->base)
      {
        pMini->m_dwRefreshTime = curTime + calcRefreshTime(pMini->base->getReliveTime(pMini->getMapID()) * MIN_T);
        pMini->m_dwDieTime = curTime;
        pMini->m_strLastKiller.clear();
        if (killer)
        {
          pMini->m_strLastKiller = killer;
        }
        if (killerId)
        {
          pMini->m_qwLastKillerID = killerId;
        }
        pMini->m_oKillerData.Clear();
        pMini->setState(BossState::WaitRefresh);

        XLOG << "[Boss], mini" << npcid << "," << pMini->base->strName << ",击杀者:" << pMini->m_strLastKiller << "(" << killerId << "),死亡时间:" << pMini->m_dwDieTime << ",下次刷新时间:" << pMini->m_dwRefreshTime << XEND;
      }
      else
      {
        XERR << "[Boss], mini" << npcid << ",击杀者:" << pMini->m_strLastKiller << "(" << killerId << "),找不到配置" << XEND;
      }
    }
    else
    {
      XERR << "[Boss], mini" << npcid << ",击杀者:" << "(" << killerId << "),找不到Boss对象" << XEND;
    }
  }
  else if(pBase->getType() == EBOSSTYPE_WORLD)
  {
    Boss* pWorld = getWorld(npcid);
    if (pWorld == nullptr)
    {
      XERR << "[Boss-击杀] map :" << mapid << "id :" << npcid << "被" << killerId << killer << "击杀,未找到该世界boss" << XEND;
      return;
    }
    pWorld->setSetTime(0);
    pWorld->setState(BossState::WaitRefresh);
    pWorld->clearWorldBoss();
    XLOG << "[Boss-击杀] map :" << mapid << "id :" << npcid << "被" << killerId << killer << "击杀, 更新状态成功, settime :" << pWorld->m_dwSetTime << XEND;
  }
}

void BossList::onEnterScene(SessionUser* pUser)
{
  if (pUser == nullptr)
    return;

  for (auto &m : m_worldList)
  {
    Boss* pWorld = m.second;
    if (pWorld->m_oWorldBossNtf.open() == false)
      continue;
    PROTOBUF(pWorld->m_oWorldBossNtf, send, len);
    pUser->sendCmdToMe(send, len);
    XDBG << "[Boss-通知]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "被通知boss状态" << pWorld->m_oWorldBossNtf.ShortDebugString() << XEND;
  }
}

void BossList::timer(DWORD cur)
{
  for (auto it=m_list.begin(); it!=m_list.end(); ++it)
  {
    if (BossState::WaitRefresh==it->second->getState())
    {
      if (it->second->m_dwRefreshTime < cur)
      {
        refresh(it->second);
      }
    }
  }

  for (auto it=m_miniList.begin(); it!=m_miniList.end(); ++it)
  {
    if (BossState::WaitRefresh==it->second->getState())
    {
      if (it->second->m_dwRefreshTime < cur)
      {
        refresh(it->second);
      }
    }
  }

  for (auto it=m_worldList.begin(); it!=m_worldList.end(); ++it)
  {
    if (BossState::WaitRefresh==it->second->getState())
    {
      if (it->second->m_dwRefreshTime < cur)
      {
        refresh(it->second);
      }
    }
  }

  m_oTimeWheel.timer(cur);
}

void BossList::refresh(Boss *boss)
{
  if (!boss || !boss->base) return;
  if (BossState::Refreshed==boss->getState()) return;
  if ((boss->base->getType() == EBOSSTYPE_DEAD || boss->base->getType() == EBOSSTYPE_WORLD) && boss->getSetTime() == 0)
    return;

  DWORD dwMapID = boss->base->getType() == EBOSSTYPE_WORLD ? boss->m_dwMapID : boss->getMapID();
  SessionScene *scene = SessionSceneManager::getMe().getSceneByID(dwMapID);
  if (scene)
  {
    Cmd::SummonBossBossSCmd message;
    message.set_mapid(dwMapID);
    message.set_npcid(boss->base->dwID);
    //message.set_bosstype(EBOSSTYPE_MVP);
    PROTOBUF(message, send, len);
    scene->sendCmd(send, len);

    boss->setState(BossState::Refreshed);
    if (boss->base->getType() != EBOSSTYPE_DEAD)
      boss->m_dwSummonTime = now();
    XLOG << "[Boss-刷新], 发送场景成功, boss:" << boss->id << "地图:" << boss->getMapID() << XEND;
  }
}

void BossList::refresh(Mini *mini)
{
  if (!mini || !mini->base) return;
  if (BossState::Refreshed==mini->getState()) return;

  SessionScene *scene = SessionSceneManager::getMe().getSceneByID(mini->getMapID());
  if (scene)
  {
    Cmd::SummonBossBossSCmd message;
    message.set_mapid(mini->getMapID());
    message.set_npcid(mini->id);
    message.set_bosstype(EBOSSTYPE_MINI);
    PROTOBUF(message, send, len);
    scene->sendCmd(send, len);

    mini->setState(BossState::Refreshed);
    XLOG << "[Mini-刷新], 发送场景成功, mini:" << mini->id << "地图:" << mini->getMapID() << XEND;
    // boss->m_dwRefreshTime = 0;
  }
}

void BossList::save()
{
  xField *bossfield = thisServer->getDBConnPool().getField(REGION_DB, "boss");
  if (bossfield)
  {
    for (auto &it : m_list)
    {
      if (!it.second) continue;

      xRecord record(bossfield);
      record.put("id", it.first);
      record.put("refresh", it.second->m_dwRefreshTime);
      record.put("dietime", it.second->m_dwDieTime);
      record.put("summontime", it.second->m_dwSummonTime);
      record.putString("killer", it.second->m_strLastKiller);
      record.put("zoneid", thisServer->getZoneID());
      record.put("killerid", it.second->m_qwLastKillerID);
      record.put("lv", it.second->getDeadLv());
      record.put("settime", it.second->getSetTime());
      record.putString("deadkiller", it.second->m_strDeadLastKiller);
      record.put("deadkillerid", it.second->m_qwDeadLastKillerID);

      thisServer->getDBConnPool().exeReplace(record);
      XLOG << "[BOSS],保存mvp,id:" << it.first << ",refresh:" << it.second->m_dwRefreshTime << ",die:" << it.second->m_dwDieTime << ",killer:" << it.second->m_strLastKiller << XEND;
    }
  }

  xField *minifield = thisServer->getDBConnPool().getField(REGION_DB, "mini");
  if (minifield)
  {
    for (auto &it : m_miniList)
    {
      if (!it.second) continue;

      xRecord record(minifield);
      record.put("id", it.second->id);
      record.put("refresh", it.second->m_dwRefreshTime);
      record.put("dietime", it.second->m_dwDieTime);
      record.putString("killer", it.second->m_strLastKiller);
      record.put("zoneid", thisServer->getZoneID());
      record.put("mapid", it.second->getMapID());
      record.put("killerid", it.second->m_qwLastKillerID);

      thisServer->getDBConnPool().exeReplace(record);
      XLOG << "[BOSS],保存mini,id:" << it.second->id << ",refresh:" << it.second->m_dwRefreshTime << ",die:" << it.second->m_dwDieTime << ",killer:" << it.second->m_strLastKiller << "map:" << it.second->getMapID() << XEND;
    }
  }
}

void BossList::load()
{
  if (thisServer->isPvpZone())
    return;

  DWORD dwNow = now();
  xField *bossfield = thisServer->getDBConnPool().getField(REGION_DB, "boss");
  if (bossfield)
  {
    char where[1024];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "zoneid=%u", thisServer->getZoneID());

    xRecordSet set;
    QWORD ret = thisServer->getDBConnPool().exeSelect(bossfield, set, where, NULL);
    if ((QWORD)-1==ret) return;

    for (DWORD i=0; i<ret; ++i)
    {
      Boss *boss = get(set[i].get<DWORD>("id"));
      if (boss)
      {
        boss->m_dwRefreshTime = set[i].get<DWORD>("refresh");
        boss->m_dwDieTime = set[i].get<DWORD>("dietime");
        boss->m_dwSetTime = set[i].get<DWORD>("settime");
        boss->m_dwSummonTime = set[i].get<DWORD>("summontime");
        boss->m_strLastKiller = set[i].getString("killer");
        boss->m_qwLastKillerID = set[i].get<QWORD>("killerid");
        boss->setSetTime(set[i].get<DWORD>("settime"));
        boss->setDeadLv(set[i].get<DWORD>("lv"));
        boss->m_qwDeadLastKillerID = set[i].get<QWORD>("deadkillerid");
        boss->m_strDeadLastKiller = set[i].getString("deadkiller");

        if (boss->getSetTime() != 0)
        {
          boss->m_dwRefreshTime = dwNow;
          boss->m_dwDieTime = boss->m_dwSummonTime + 1;
        }
        else if (boss->m_dwRefreshTime == 0)
          boss->m_dwRefreshTime = dwNow;

        XLOG << "[Boss-加载] bossid :" << boss->id << "加载成功, refreshtime :" << boss->m_dwRefreshTime << "dietime :" << boss->m_dwDieTime << "settime :" << boss->m_dwSetTime
          << "killer :" << boss->m_strLastKiller << "killerid :" << boss->m_qwLastKillerID << "deadkiller :" << boss->m_strDeadLastKiller << "deadkillerid :" << boss->m_qwDeadLastKillerID << XEND;

        if (boss->getSetTime() != 0)
        {
          const SBossCFG* pCFG = BossConfig::getMe().getBossCFG(boss->id);
          if (pCFG == nullptr)
          {
            XERR << "[Boss-加载] bossid :" << boss->id << "被点名,变换亡者boss失败,该boss未在 Table_Boss.txt 表中找到" << XEND;
            continue;
          }
          const SBossCFG* pDeadCFG = BossConfig::getMe().getBossCFG(pCFG->dwDeadID);
          if (pDeadCFG == nullptr)
          {
            XERR << "[Boss-加载] bossid :" << boss->id << "被点名,变换亡者boss失败,亡者boss :" << pCFG->dwDeadID << "未在 Table_Boss.txt 表中找到" << XEND;
            continue;
          }

          boss->setDieTime(dwNow);
          boss->base = pDeadCFG;
          XLOG << "[Boss-加载] bossid :" << boss->id << "被点名,变换亡者boss :" << boss->base->dwID << "成功" << XEND;
        }
        /*
        if (boss->m_dwDieTime > boss->m_dwRefreshTime)
        {
          boss->setState(BossState::WaitRefresh);
        }
        */
      }
    }
  }

  xField *minifield = thisServer->getDBConnPool().getField(REGION_DB, "mini");
  if (minifield)
  {
    char where[1024];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "zoneid=%u", thisServer->getZoneID());

    xRecordSet set;
    QWORD ret = thisServer->getDBConnPool().exeSelect(minifield, set, where, NULL);
    if ((QWORD)-1==ret) return;

    for (DWORD i=0; i<ret; ++i)
    {
      Mini *mini = get(set[i].get<DWORD>("id"), set[i].get<DWORD>("mapid"));
      if (mini)
      {
        mini->m_dwRefreshTime = set[i].get<DWORD>("refresh");
        mini->m_dwDieTime = set[i].get<DWORD>("dietime");
        mini->m_strLastKiller = set[i].getString("killer");
        mini->m_qwLastKillerID = set[i].get<QWORD>("killerid");
        XLOG << "[BOSS],加载mini,id:" << mini->id << ",map:" << mini->getMapID() << ",refresh:" << mini->m_dwRefreshTime << ",die:" << mini->m_dwDieTime << ",killer:" << mini->m_strLastKiller << XEND;
      }
    }
  }
}

void BossList::updateWorldBoss(const WorldBossNtfBossSCmd& cmd)
{
  for (auto &m : m_worldList)
  {
    Boss* pWorld = m.second;
    if (pWorld->m_dwMapID == cmd.ntf().mapid())
    {
      m.second->updateWorldBoss(cmd.ntf());
      XDBG << "[Boss-世界boss] mapid :" << pWorld->m_dwMapID << "bossid :" << pWorld->id << "收到通知" << cmd.ntf().ShortDebugString() << XEND;
      break;
    }
  }
}

