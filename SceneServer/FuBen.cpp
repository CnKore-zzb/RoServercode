#include "FuBen.h"
#include "DScene.h"
#include "SceneUser.h"
#include "SceneNpc.h"
#include "SceneNpcManager.h"
#include "SceneUserManager.h"
#include "xTools.h"
#include "FuBenCmd.pb.h"
#include "MsgManager.h"
#include "GMCommandRuler.h"
#include "SceneManager.h"
#include "NpcConfig.h"
#include "CarrierCmd.pb.h"
#include "SessionTeam.pb.h"
#include "SceneServer.h"
#include "TowerConfig.h"
#include "GuidManager.h"
#include "PlatLogManager.h"
#include "MapConfig.h"
#include "DojoConfig.h"

std::map<DWORD, FuBenEntry> FuBen::s_cfg;

FuBen::FuBen(DScene *qscene):m_pQuestScene(qscene)
{
  clear();
  m_bCheckAgain = false;
}

FuBen::~FuBen()
{
  //final();
}

bool FuBen::appendPhase(FuBenEntry & rFuBenEntry)
{
  for (auto it = rFuBenEntry.begin(); it != rFuBenEntry.end(); ++it)
  {
    auto v = m_phase.find(it->first);
    if (v != m_phase.end())
    {
      FuBenPhaseList& rList = v->second;
      for (auto m = it->second.begin(); m != it->second.end(); ++m)
      {
        rList.push_back(*m);
      }
    }
    else
    {
      m_phase.insert(make_pair(it->first, it->second));
    }
  }
  return true;
}

void FuBen::final()
{
  for (auto it=s_cfg.begin(); it!=s_cfg.end(); ++it)
  {
    FuBenEntry &entry = it->second;
    for (auto iter=entry.begin(); iter!=entry.end(); ++iter)
    {
      std::list<FuBenPhase *> &list = iter->second;
      for (auto p_iter=list.begin(); p_iter!=list.end(); ++p_iter)
        SAFE_DELETE(*p_iter);
    }
  }
  s_cfg.clear();
}

//重新加载会有内存泄露 避免宕机 不处理
bool FuBen::loadConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_Raid.txt"))
  {
    XERR << "[表格],加载表格,Table_Raid.txt,失败" << XEND;
    return false;
  }

  bool bCorrect = true;
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Raid", table);
  for (auto it = table.begin(); it != table.end(); ++it)
  {
    xLuaData &data = it->second;

    int fubenID = data.getTableInt("RaidID");
    if (MapConfig::getMe().getRaidCFG(fubenID) == nullptr)
    {
      XERR << "[副本配置-加载] id :" << it->first << "RaidID :" << fubenID << "未在 Table_MapRaid.txt 表中找到" << XEND;
      bCorrect = false;
      continue;
    }
    int starID = data.getTableInt("StarID");
    if (!starID)
    {
      XERR << "[副本配置-加载] id :" << it->first << "StarID :" << starID << "不合法" << XEND;
      bCorrect = false;
      continue;
    }

    FuBenPhase *phase = nullptr;
    const char *type = data.getTableString("Content");
    if (0==strcmp(type, "story"))
      phase = NEW StoryFuBenPhase(data);
    else if (0==strcmp(type, "rush"))
      phase = NEW RushFuBenPhase(data);
    else if (0==strcmp(type, "trace"))
      phase = NEW TrackFuBenPhase(data);
    else if (0==strcmp(type, "killAll"))
      phase = NEW KillAllFuBenPhase(data);
    else if (0==strcmp(type, "reward"))
      phase = NEW RewardFuBenPhase(data);
    else if (0==strcmp(type, "summon"))
      phase = NEW SummonFuBenPhase(data);
    else if (0==strcmp(type, "win"))
      phase = NEW WinFuBenPhase(data);
    else if (0==strcmp(type, "lose"))
      phase = NEW LoseFuBenPhase(data);
    else if (0==strcmp(type, "leave"))
      phase = NEW LeaveFuBenPhase(data);
    else if (0==strcmp(type, "kill"))
      phase = NEW KillFuBenPhase(data);
    else if (0==strcmp(type, "clearNpc"))
      phase = NEW ClearNpcFuBenPhase(data);
    else if (0==strcmp(type, "wait"))
      phase = NEW TimerFuBenPhase(data);
    else if (0==strcmp(type, "close"))
      phase = NEW CloseFuBenPhase(data);
    else if (0==strcmp(type, "visit"))
      phase = NEW VisitFuBenPhase(data);
    else if (0==strcmp(type, "star"))
      phase = NEW StarFuBenPhase(data);
    else if (0==strcmp(type, "GM"))
      phase = NEW GMCmdFuBenPhase(data);
    else if (0==strcmp(type, "move"))
      phase = NEW MoveFuBenPhase(data);
    else if (0==strcmp(type, "wait_gear"))
      phase = NEW GearFuBenPhase(data);
    else if (0==strcmp(type, "wait_star"))
      phase = NEW WaitStarFuBenPhase(data);
    else if (0==strcmp(type, "append"))
      phase = NEW AppendFuBenPhase(data);
    else if (0==strcmp(type, "nextlayer"))
      phase = NEW EnterLayerPhase(data);
    else if (0==strcmp(type, "towerreward"))
      phase = NEW TowerRewardPhase(data);
    else if (0==strcmp(type, "towertimer"))
      phase = NEW TowerTimerPhase(data);
    else if (0==strcmp(type, "tower_summon"))
      phase = NEW TowerSummonPhase(data);
    else if (0==strcmp(type, "tower_killAll"))
      phase = NEW TowerKillAllPhase(data);
    else if (0==strcmp(type, "playernum"))
      phase = NEW PlayerNumPhase(data);
    else if (0==strcmp(type, "carrier"))
      phase = NEW CarrierFuBenPhase(data);
    else if (0==strcmp(type, "timerdown"))
      phase = NEW TimerDownPhase(data);
    else if (0==strcmp(type, "laboratory"))
      phase = NEW LaboratoryMonsterPhase(data);
    else if (0==strcmp(type, "monster_count"))
      phase = NEW MonsterCountPhase(data);
    else if (0 == strcmp(type, "DojoReward"))
      phase = NEW DojoRewardPhase(data);
    else if (0 == strcmp(type, "DojoSummon"))
      phase = NEW DojoSummonPhase(data);
    else if (0==strcmp(type, "beattack"))
      phase = NEW BeAttackPhase(data);
    else if (0==strcmp(type, "MultiGM"))
      phase = NEW MultiGMFuBenPhase(data);
    else if (0==strcmp(type, "dialog"))
      phase = NEW DialogFuBenPhase(data);
    else if (0==strcmp(type, "rand_summon"))
      phase = NEW RandSummonFuBenPhase(data);
    else if (0==strcmp(type, "collect"))
      phase = NEW CollectFuBenPhase(data);
    else if (0==strcmp(type, "client_plot"))
      phase = NEW ClientPlotFuBenPhase(data);
    else if (0==strcmp(type, "use"))
      phase = NEW UseFuBenPhase(data);
    else if (0==strcmp(type, "npcgm"))
      phase = NEW NpcGMFuBenPhase(data);
    else if (0==strcmp(type, "seal_reward"))
      phase = NEW SealRewardFuBenPhase(data);
    else if (0==strcmp(type, "guild_raid_summon"))
      phase = NEW GRaidSummonFuBenPhase(data);
    else if (0==strcmp(type, "guild_raid_reward"))
      phase = NEW GRaidRewardFuBenPhase(data);
    else if (0==strcmp(type, "guild_fire_check_monster"))
      phase = NEW GuildFireCheckMonster(data);
    else if (0==strcmp(type, "summon_metal_npc"))
      phase = NEW GuildFireSummonMetal(data);
    else if (0==strcmp(type, "uniq_rand_summon"))
      phase = NEW UniqRandSummonFuBenPhase(data);
    else if (0==strcmp(type, "altmanreward"))
      phase = NEW AltmanRewardPhase(data);
    else if (0==strcmp(type, "check_summon_deadboss"))
      phase = NEW CheckSummonDeadBossNpc(data);
    else
    {
      XERR << "[副本配置-加载] id :" << it->first << "Content :" << type << "无法识别的类型" << XEND;
      bCorrect = false;
      continue;
    }

    if (phase == nullptr)
    {
      XERR << "[副本配置-加载] id :" << it->first << "Content :" << type << "创建失败" << XEND;
      bCorrect = false;
      continue;
    }

    phase->setID(it->first);
    if (phase->init() == false)
    {
      bCorrect = false;
      SAFE_DELETE(phase);
      continue;
    }

    s_cfg[fubenID][starID].push_back(phase);
  }

  if (bCorrect)
    XLOG << "[副本配置-加载] 成功加载 Table_Raid.txt 表, 总共 :" << s_cfg.size() << "个" << XEND;
  return bCorrect;
}

void FuBen::timer(QWORD msec)
{
  check("wait");
  check("timerdown");
  check("beattack");

  if (TowerConfig::getMe().isTower(m_pQuestScene->getMapID()) == true)
  {
    if (!m_bCheckAgain)
    {
      check("tower_killAll");
      check("towertimer");
    }
    else
    {
      check();
      m_bCheckAgain = false;
    }
  }

  DWORD sec = msec / 1000;
  switch (m_pQuestScene->getFuben().m_result)
  {
    case FUBEN_RESULT_FAIL:
      {

      }
      break;
    case FUBEN_RESULT_NULL:
      {
        if (m_pQuestScene->getCloseTime())
        {
          if (sec > m_pQuestScene->getCloseTime())
          {
            fail();
          }
        }
      }
      break;
    case FUBEN_RESULT_SUCCESS:
      {

      }
      break;
    default:
      break;
  }
}

void FuBen::check()
{
  if (FUBEN_RESULT_FAIL==m_result) return;
  for (auto it=m_phase.begin(); it!=m_phase.end(); ++it)
  {
    check(it->second);
  }
}

void FuBen::check(FuBenPhaseList &list)
{
  if (FUBEN_RESULT_FAIL==m_result) return;
  auto it = list.begin();
  auto tmp = it;
  for ( ; it!=list.end(); )
  {
    tmp = it++;
    if (*tmp)
    {
    //  if (FUBEN_RESULT_SUCCESS==m_result)
     // {
      //  if ((*tmp)->getStarID()!=1) continue;
     // }
      if ((*tmp)->isNotify())
      {
        syncStep(*tmp);
        setID((*tmp)->getStarID(), (*tmp)->getID());
      }
      if ((*tmp)->isType("use") || (*tmp)->isType("wait") || (*tmp)->isType("visit") || (*tmp)->isType("killAll") || (*tmp)->isType("towertimer") ||
          (*tmp)->isType("tower_killAll") || (*tmp)->isType("timerdown") || (*tmp)->isType("dialog"))
      {
        return;
      }
      if (m_blCheck) return;
      m_blCheck = true;
      if ((*tmp)->exec(m_pQuestScene))
      {
        XDBG << "[副本],exec:" << (*tmp)->getStarID() << (*tmp)->getTypeString() << XEND;

        //避免story提前结束
        if ((*tmp)->isType("story"))
        {
          return;
        }
        list.erase(tmp);
        m_blCheck = false;
      }
      else
      {
        /*if ((*tmp)->isSyncClient())
        {
          syncStep((*tmp)->getID());
        }*/
        m_blCheck = false;
        return;
      }
    }
  }
}

void FuBen::check(const std::string& t, DWORD starID)
{
  if (FUBEN_RESULT_FAIL==m_result) return;
  if (m_phase.empty()) return;

  for (auto it=m_phase.begin(); it!=m_phase.end(); ++it)
  {
    if (starID && starID!=it->first) continue;

    check(it->second, t);
  }
}

void FuBen::check(FuBenPhaseList &list, const std::string &t)
{
  if (FUBEN_RESULT_FAIL==m_result) return;
  if (!list.empty())
  {
    auto it = list.begin();
    if (*it && ((*it)->isType(t)))
    {
      if ((*it)->isNotify())
      {
        setID((*it)->getStarID(), (*it)->getID());
      }
      if (m_blCheck) return;
      m_blCheck = true;
      if ((*it)->exec(m_pQuestScene))
      {
#ifdef _ALL_SUPER_GM
        XLOG << "[FuBen],exec:" << (*it)->getStarID() << (*it)->getTypeString() << XEND;
#endif
#ifdef __TYH_TOWER_DEBUG
        XLOG << "[FuBen],exec:" << (*it)->getStarID() << (*it)->getTypeString() << XEND;
#endif
        list.erase(it);
        m_blCheck = false;
        check();
      }
      m_blCheck = false;
    }
  }
}

void FuBen::check(DWORD id)
{
  if (FUBEN_RESULT_FAIL==m_result) return;
  if (m_phase.empty()) return;

  for (auto it=m_phase.begin(); it!=m_phase.end(); ++it)
  {
    if (it->second.empty())
      continue;
    auto s = it->second.begin();
    if (*s && (*s)->getID() == id)
    {
      check((*it->second.begin())->getTypeString());
      break;
    }
  }
}

void FuBen::enter(SceneUser *user)
{
  if (!user) return;

  for (auto &m : m_phase)
  {
    for (auto &l : m.second)
    {
      if (l->isNotify())
        syncStep(l);
      break;
    }
  }

  //sendInfo(user);
  check();

  XLOG << "[新副本]" << user->id << user->name << "进入:" << m_pQuestScene->name << XEND;
}

void FuBen::sendInfo(SceneUser *user)
{
  Cmd::TrackFuBenUserCmd message;
  message.set_dmapid(m_pQuestScene->getRaidID());
  message.set_endtime(m_pQuestScene->getCloseTime() - 10);
  for (auto it=m_oVars.begin(); it!=m_oVars.end(); ++it)
  {
    Cmd::TrackData *pData = message.add_data();
    pData->set_star(it->first);
    pData->set_id(it->second.m_dwID);
    XLOG << "[新副本],star:" << it->first << "id:" << it->second.m_dwID << XEND;
  }
  PROTOBUF(message, send, len);

  if (user)
  {
    user->sendCmdToMe(send, len);
  }
  else
  {
    MsgManager::sendMapCmd(m_pQuestScene->id, send, len);
  }
}

void FuBen::setID(DWORD star, DWORD id)
{
  DWORD old = m_oVars[star].m_dwID;
  if (id!=old)
  {
    m_oVars[star].m_dwID = id;
    sendInfo(NULL);
  }
}

void FuBen::leave(SceneUser *user)
{
  if (!user) return;

  if (m_result == FUBEN_RESULT_NULL)
  {
    DScene* pScene = dynamic_cast<DScene*>(user->getScene());
    if (pScene != nullptr && pScene->isQuestFail() == true)
      user->getEvent().onPassRaid(pScene->getRaidID(), false);
  }

  //user->changeHp(user->getAttr(EATTRTYPE_MAXHP));
  // 离开副本时通知, 客户端删除副本步骤
  FuBenClearInfoCmd cmd;
  PROTOBUF(cmd, send, len);
  user->sendCmdToMe(send, len);
  /*
  for (auto it=m_phase.begin(); it!=m_phase.end(); ++it)
  {
    if (!it->second.empty())
    {
      FuBenClearInfoCmd cmd;
      PROTOBUF(cmd, send, len);
      user->sendCmdToMe(send, len);
    }
  }
  */
}

void FuBen::die(SceneUser *user)
{
  if (!user) return;

  fail();
}

void FuBen::fail()
{
  if (FUBEN_RESULT_SUCCESS != m_result)
  {
    Cmd::FailFuBenUserCmd message;
    PROTOBUF(message, send, len);
    //Chat::sendCmdArea(m_pQuestScene->id, send, len);
    m_result = FUBEN_RESULT_FAIL;

    //m_pQuestScene->setCloseTime(now() + m_pQuestScene->m_base->dwRaidEndWait);
  }
}

bool FuBen::add(DWORD fuben_id)
{
  m_pQuestScene->setCloseTime(now() + m_pQuestScene->getRaidLimitTime());
  clear();
  if (s_cfg.find(fuben_id)!=s_cfg.end())
  {
    m_phase.clear();
    m_phase = s_cfg[fuben_id];
    XDBG << "[FuBen],add:" << fuben_id << XEND;
    return true;
  }
  return false;
}

void FuBen::clear()
{
  m_oVars.clear();
  m_result = FUBEN_RESULT_NULL;
}

/*bool FuBen::isStarFinish(DWORD starID)
{
  if (!starID) return true;

  for (auto it=m_phase.begin(); it!=m_phase.end(); ++it)
  {
    if (starID!=it->first) continue;

    if (it->second.empty()) return true;
    return false;
  }
  return true;
}*/

void FuBen::syncStep(FuBenPhase* pPhase, SceneUser* user /*= nullptr*/)
{
  if (pPhase == nullptr)
    return;

  FubenStepSyncCmd cmd;
  cmd.set_id(pPhase->getID());
  pPhase->initPConfig(cmd.mutable_config());

  std::map<DWORD, FuBenEntry> s_cfg;

  PROTOBUF(cmd, send, len);
  if (user)
  {
    user->sendCmdToMe(send, len);
  }
  else
  {
    MsgManager::sendMapCmd(m_pQuestScene->id, send, len);
  }

  XDBG << "[副本] 同步 id :" << pPhase->getID() << XEND;
}

bool FuBen::clearRaid()
{
  if (m_pQuestScene == nullptr)
    return false;
  const SceneObject* pObj = m_pQuestScene->getSceneObject();
  if (pObj == nullptr)
    return false;
  const map<DWORD, SceneNpcTemplate>& raidnpclist = pObj->getRaidNpcList();
  xSceneEntrySet entrySet;
  m_pQuestScene->getAllEntryList(SCENE_ENTRY_NPC, entrySet);
  for (auto &s : entrySet)
  {
    SceneNpc* npc = dynamic_cast<SceneNpc*> (s);
    if (npc == nullptr)
      continue;
    if (npc->define.getUniqueID() && raidnpclist.find(npc->define.getUniqueID()) != raidnpclist.end())
    {
      npc->removeAtonce();
      XLOG << "[副本], 清空, 移除怪物" << npc->name << npc->id << npc->define.getUniqueID() << "副本:" << m_pQuestScene->id << XEND;
    }
  }

  // 防止怪物再次复活
  const std::set<SceneNpc *>& deaths = SceneNpcManager::getMe().getLeaveNpcs();
  for (auto &s : deaths)
  {
    if (s->getSceneID() == m_pQuestScene->id)
      s->setStatus(ECREATURESTATUS_CLEAR);
  }

  clear();
  m_phase.clear();
  XLOG << "[副本], 清空, 地图:" << m_pQuestScene->id << "副本ID:" << m_pQuestScene->getRaidID() << XEND;
  return true;
}

bool FuBen::restart()
{
  check();
  return true;
}

