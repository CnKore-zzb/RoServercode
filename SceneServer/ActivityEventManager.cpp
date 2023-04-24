#include "ActivityEventManager.h"
#include "SceneManager.h"
#include "SceneNpcManager.h"
#include "SceneNpc.h"

void ActivityEventManager::updateEvent(const ActivityEventNtfSessionCmd& cmd)
{
  m_mapType2Event.clear();

  for (int i = 0; i < cmd.infos_size(); ++i)
  {
    map<QWORD, ActivityEventInfo>& events = m_mapType2Event[cmd.infos(i).type()];
    auto it = events.find(cmd.infos(i).id());
    if (it == events.end())
    {
      //只有最后一个活动生效
      if (isOnlyOne(cmd.infos(i).type()))
        events.clear();

      events[cmd.infos(i).id()] = cmd.infos(i);
    }
    else
      events[cmd.infos(i).id()].CopyFrom(cmd.infos(i));
  }
  XLOG << "[活动模板-数据更新] 成功" << XEND;

  summonNpc();
  summonLotteryNpc();
  updateShopCache();
}

// 地图传送免费
bool ActivityEventManager::isTransferToMapFree(EGoToGearType type, DWORD mapid)
{
  auto it = m_mapType2Event.find(EACTIVITYEVENTTYPE_FREE_TRANSFER);
  if (it == m_mapType2Event.end())
    return false;

  for (auto& v : it->second)
  {
    if (isEventStart(v.second) == false)
      continue;
    switch (type)
    {
    case EGoToGearType_Single:
    case EGoToGearType_Hand:
    {
      if (v.second.freetransferinfo().allfree())
        return true;
      for (int i = 0; i < v.second.freetransferinfo().mapids_size(); ++i)
        if (v.second.freetransferinfo().mapids(i) == mapid)
          return true;
      continue;
    }
    case EGoToGearType_Team:
    {
      if (v.second.freetransferinfo().teamallfree())
        return true;
      for (int i = 0; i < v.second.freetransferinfo().teammapids_size(); ++i)
        if (v.second.freetransferinfo().teammapids(i) == mapid)
          return true;
      continue;
    }
    default:
      continue;
    }
  }
  return false;
}

// 仓库使用免费
bool ActivityEventManager::isStoreFree()
{
  auto it = m_mapType2Event.find(EACTIVITYEVENTTYPE_FREE_TRANSFER);
  if (it == m_mapType2Event.end())
    return false;

  for (auto& v : it->second)
  {
    if (isEventStart(v.second) == false)
      continue;
    if (v.second.freetransferinfo().storefree())
      return true;
  }
  return false;
}

// 额外奖励/多倍奖励
bool ActivityEventManager::getReward(SceneUser* user, EAERewardMode mode, DWORD count, TVecItemInfo& items, DWORD& times)
{
  if (user == nullptr || count == 0)
    return false;

  auto it = m_mapType2Event.find(EACTIVITYEVENTTYPE_REWARD);
  if (it == m_mapType2Event.end())
    return false;

  map<DWORD, DWORD> id2count;
  for (auto& v : it->second)
  {
    if (isEventStart(v.second) == false || v.second.rewardinfo().mode() != mode)
      continue;
    AERewardInfo* rewardinfo = v.second.mutable_rewardinfo();
    if (rewardinfo == nullptr)
      return false; // 同时只能有一个开启的奖励活动

    if (rewardinfo->has_extrareward())
    {
      AERewardExtraInfo* info = rewardinfo->mutable_extrareward();

      // 绑定角色, 只要完成一次立马绑定
      bindCharExtra(mode, info, user);

      if (info && canGetRewardExtra(mode, info, user, count))
      {
        DWORD total_weight = 0;
        map<DWORD, AEReward*> weight2item;
        for (int i = 0; i < info->rewards_size(); ++i)
        {
          AEReward* reward = info->mutable_rewards(i);
          if (reward && reward->id() != 0 && reward->count() != 0)
          {
            switch (mode)
            {
            case EAEREWARDMODE_TOWER: // 无限塔, count为当前层数, 额外奖励指定层数掉落
            {
              if ((reward->minlayer() && count < reward->minlayer()) ||
                  (reward->maxlayer() && count > reward->maxlayer()))
                continue;
              break;
            }
            case EAEREWARDMODE_PVECARD: // 卡片副本, count为副本难道, 额外奖励指定难道掉落
            {
              if (reward->difficultys_size() > 0)
              {
                int i = 0;
                for (; i < reward->difficultys_size(); ++i)
                  if (count == reward->difficultys(i))
                    break;
                if (i >= reward->difficultys_size())
                  continue;
              }
              break;
            }
            default:
              break;
            }

            if (reward->weight() == 0) // 必掉道具
            {
              auto m = id2count.find(reward->id());
              if (m == id2count.end())
                id2count[reward->id()] = 0;
              id2count[reward->id()] += reward->count();
            }
            else
            {
              total_weight += reward->weight();
              weight2item[total_weight] = reward;
            }
          }
        }

        // 按权重掉落
        if (weight2item.empty() == false)
        {
          DWORD rand = randBetween(1, total_weight);
          for (auto& m : weight2item)
            if (rand <= m.first)
            {
              auto s = id2count.find(m.second->id());
              if (s == id2count.end())
                id2count[m.second->id()] = 0;
              id2count[m.second->id()] += m.second->count();
              break;
            }
        }

        XLOG << "[活动模板-额外奖励]" << user->accid << user->id << user->name << "mode:" << mode << "count:" << count << "奖励:";
        TVecItemInfo rewards;
        for (auto& v : id2count)
        {
          ItemInfo item;
          item.set_id(v.first);
          item.set_count(v.second);
          item.set_source(ESOURCE_ACTIVITY_EVENT);
          rewards.push_back(item);
          XLOG << v.first << v.second;
        }
        combinItemInfo(items, rewards);
        XLOG << "获得额外奖励成功" << XEND;

        if (info->daylimit())
          user->getUserSceneData().addAERewardDayCount(rewardinfo->mode(), 1);
      }
    }

    if (rewardinfo->has_multiplereward())
    {
      AERewardMultipleInfo* info = rewardinfo->mutable_multiplereward();

      // 绑定角色, 只要完成一次立马绑定
      bindCharMulti(mode, info, user);

      if (info && info->multiple() > 1 && canGetRewardMutiple(mode, info, user, count))
      {
        times = info->multiple();
        XLOG << "[活动模板-多倍奖励]" << user->accid << user->id << user->name << "mode:" << mode << "count:" << count << "倍率:" << info->multiple() << "获得多倍奖励成功" << XEND;

        if (info->daylimit())
          user->getUserSceneData().addAERewardMulDayCount(rewardinfo->mode(), 1);
      }
    }

    return true;
  }

  return false;
}

bool ActivityEventManager::canGetRewardExtra(EAERewardMode mode, AERewardExtraInfo* info, SceneUser* user, DWORD finishcount)
{
  if (user == nullptr || info == nullptr || finishcount <= 0)
    return false;

  // 1、完成次数：每完成多少次可以获得一次奖励
  if (info->finishcount() && finishcount % info->finishcount() != 0)
    return false;

  // 2、每天能获得多少次奖励
  if (info->daylimit() && user->getUserSceneData().getAERewardDayCount(mode) >= info->daylimit())
    return false;

  // 3、账号限制：账号下只有一个角色能够获得当天的奖励
  if (info->acclimit())
  {
    QWORD charid = user->getUserSceneData().getAERewardAccLimitCharID(mode);
    if (charid && user->id != charid)
      return false;
  }

  return true;
}

bool ActivityEventManager::canGetRewardMutiple(EAERewardMode mode, AERewardMultipleInfo* info, SceneUser* user, DWORD finishcount)
{
  if (user == nullptr || info == nullptr || finishcount <= 0)
    return false;

  // 1、完成次数：每完成多少次可以获得一次奖励
  if (info->finishcount() && finishcount % info->finishcount() != 0)
    return false;

  // 2、每天能获得多少次奖励
  if (info->daylimit() && user->getUserSceneData().getAERewardMulDayCount(mode) >= info->daylimit())
    return false;

  // 3、账号限制：账号下只有一个角色能够获得当天的奖励
  if (info->acclimit())
  {
    QWORD charid = user->getUserSceneData().getAERewardMulAccLimitCharID(mode);
    if (charid && user->id != charid)
      return false;
  }

  return true;
}

void ActivityEventManager::bindCharExtra(EAERewardMode mode, AERewardExtraInfo* info, SceneUser* user)
{
  if (user == nullptr || info == nullptr)
    return;
  if (info->acclimit() && user->getUserSceneData().getAERewardAccLimitCharID(mode) <= 0)
    user->getUserSceneData().setAERewardAccLimitCharID(mode, user->id);
}

void ActivityEventManager::bindCharMulti(EAERewardMode mode, AERewardMultipleInfo* info, SceneUser* user)
{
  if (user == nullptr || info == nullptr || info->multiple() <= 1)
    return;
  if (info->acclimit() && user->getUserSceneData().getAERewardMulAccLimitCharID(mode) <= 0)
    user->getUserSceneData().setAERewardMulAccLimitCharID(mode, user->id);
}

void ActivityEventManager::summonNpc()
{
  bool clearall = false;
  auto it = m_mapType2Event.find(EACTIVITYEVENTTYPE_SUMMON);
  if (it != m_mapType2Event.end())
  {
    for (auto& v : it->second)
    {
      if (isEventStart(v.second) == false || m_mapID2NpcID.find(v.second.id()) != m_mapID2NpcID.end())
        continue;

      const AESummonInfo& info = v.second.summoninfo();

      NpcDefine def;
      def.setID(info.monsterid());
      def.setLife(info.revive() ? 0 : 1);
      for (int i = 0; i < info.rewards_size(); ++i)
      {
        if (info.rewards(i).id() <= 0 || info.rewards(i).count() <= 0)
          XERR << "[活动模板-召唤怪物] 活动id:" << v.second.id() << "id:" << info.rewards(i).id() << "count:" << info.rewards(i).count() << "奖励配置错误" << XEND;
        else
          def.addRandomReward(info.rewards(i).id(), info.rewards(i).count(), info.rewards(i).weight());
      }

      if (info.iscreate())
      {
        def.setName(info.namezh().c_str());
        def.setIcon(info.icon());   // todo
        def.setRaceType(NpcConfig::getMe().getRaceType(info.race()));
        def.setNatureType(NpcConfig::getMe().getNatureType(info.nature()));
        def.setShape(NpcConfig::getMe().getShape(info.shape()));
        def.setBody(info.body());
        def.setJobExp(info.jobexp());
        def.setBaseExp(info.baseexp());
        def.setScaleMin(info.scale());
        def.setScaleMax(info.scale());
        def.setNormalSkillID(info.normalskill());
      }

      for (int i = 0; i < info.map_size(); ++i)
      {
        Scene* scene = SceneManager::getMe().getSceneByID(info.map(i).id());
        if (scene == nullptr)
        {
          XERR << "[活动模板-召唤怪物] 活动id:" << v.second.id() << "地图id:" << info.map(i).id() << "场景找不到" << XEND;
          continue;
        }

        XLOG << "[活动模板-召唤怪物] 活动id:" << v.second.id() << "id:" << def.getID() << "map:" << info.map(i).id() << "count:" << info.count() << XEND;

        for (int j = 0; j < info.map(i).coord_size(); ++j)
        {
          if (info.map(i).coord(i).pos_size() != 3)
          {
            XERR << "[活动模板-召唤怪物] 活动id:" << v.second.id() << "地图id:" << info.map(i).id()  << "npcid:" << def.getID() << "坐标错误" << XEND;
            continue;
          }
          def.setPos(xPos(info.map(i).coord(i).pos(0), info.map(i).coord(i).pos(1), info.map(i).coord(i).pos(2)));
          def.setRange(info.map(i).coord(i).range());

          for (DWORD k = 0; k < info.count(); ++k)
          {
            SceneNpc* npc = SceneNpcManager::getMe().createNpc(def, scene);
            if (npc == nullptr)
            {
              XERR << "[活动模板-召唤怪物] 活动id:" << v.second.id() << "地图id:" << info.map(i).id()  << "npcid:" << def.getID() << "创建失败" << XEND;
              continue;
            }

            npc->setGMAttr(EATTRTYPE_STR, info.str()); // 力量
            npc->setGMAttr(EATTRTYPE_DEX, info.dex()); // 灵巧
            npc->setGMAttr(EATTRTYPE_INT, info.inte()); // 智力
            npc->setGMAttr(EATTRTYPE_VIT, info.vit()); // 体质
            npc->setGMAttr(EATTRTYPE_AGI, info.agi()); // 敏捷
            npc->setGMAttr(EATTRTYPE_LUK, info.luk()); // 幸运
            npc->setGMAttr(EATTRTYPE_ATK, info.atk()); // 物理攻击
            npc->setGMAttr(EATTRTYPE_MATK, info.matk()); // 魔法攻击
            npc->setGMAttr(EATTRTYPE_DEF, info.def()); // 物理防御
            npc->setGMAttr(EATTRTYPE_MDEF, info.mdef()); // 魔法防御
            npc->setGMAttr(EATTRTYPE_HP, info.hp()); // 生命值
            npc->setGMAttr(EATTRTYPE_HIT, info.hit()); // 命中
            npc->setGMAttr(EATTRTYPE_FLEE, info.flee()); // 闪避
            npc->setGMAttr(EATTRTYPE_MOVESPD, info.movespd()); // 移动速度
            npc->setGMAttr(EATTRTYPE_MOVESPDPER, info.movespdrate()); // 移动倍率

            m_mapID2NpcID[v.second.id()].insert(npc->getTempID());
          }
        }
      }
    }
  }
  else
  {
    clearall = true;
  }

  for (auto v = m_mapID2NpcID.begin(); v != m_mapID2NpcID.end();)
  {
    if (clearall || it->second.find(v->first) == it->second.end())
    {
      XLOG << "[活动模板-召唤怪物] 活动id:" << v->first << "怪物清除" << XEND;
      for (auto id : v->second)
      {
        SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(id);
        if (npc)
          npc->removeAtonce();
      }
      v = m_mapID2NpcID.erase(v);
      continue;
    }
    ++v;
  }
}

void ActivityEventManager::summonLotteryNpc()
{
  bool clearall = false;
  auto it = m_mapType2Event.find(EACTIVITYEVENTTYPE_LOTTERY_NPC);
  if (it != m_mapType2Event.end())
  {
    for (auto& v : it->second)
    {
      //只修改地图坐标是不会再召npc的的
      if (isEventStart(v.second) == false || m_mapID2LotteryNpcID.find(v.second.id()) != m_mapID2LotteryNpcID.end())
        continue;

      const AELotteryNpc& info = v.second.lotterynpc();
      DWORD dwNpcId = getLotteryNpcId(info.lotterytype());
      if (dwNpcId == 0)
        continue;

      NpcDefine def;
      def.setID(dwNpcId);
      for (int i = 0; i < info.map_size(); ++i)
      {
        Scene* scene = SceneManager::getMe().getSceneByID(info.map(i).id());
        if (scene == nullptr)
        {
          XERR << "[活动模板-扭蛋NPC-召唤] 活动id:" << v.second.id() << "地图id:" << info.map(i).id() << "场景找不到" << XEND;
          continue;
        }

        XLOG << "[活动模板-扭蛋NPC-召唤] 活动id:" << v.second.id() << "id:" << def.getID() << "map:" << info.map(i).id() <<XEND;

        for (int j = 0; j < info.map(i).coord_size(); ++j)
        {
          if (info.map(i).coord(i).pos_size() != 3)
          {
            XERR << "[活动模板-扭蛋NPC-召唤] 活动id:" << v.second.id() << "地图id:" << info.map(i).id() << "npcid:" << def.getID() << "坐标错误" << XEND;
            continue;
          }
          def.setPos(xPos(info.map(i).coord(i).pos(0), info.map(i).coord(i).pos(1), info.map(i).coord(i).pos(2)));
          def.setRange(info.map(i).coord(i).range());

          SceneNpc* npc = SceneNpcManager::getMe().createNpc(def, scene);
          if (npc == nullptr)
          {
            XERR << "[活动模板-扭蛋NPC-召唤] 活动id:" << v.second.id() << "地图id:" << info.map(i).id() << "npcid:" << def.getID() << "创建失败" << XEND;
            continue;
          }
          m_mapID2LotteryNpcID[v.second.id()].insert(npc->getTempID());
        }
      }
    }
  }
  else
    clearall = true;

  for (auto v = m_mapID2LotteryNpcID.begin(); v != m_mapID2LotteryNpcID.end();)
  {
    if (clearall || isEventStop(v->first, it->second))
    {
      XLOG << "[活动模板-扭蛋NPC] 活动id:" << v->first << "怪物清除" << XEND;
      for (auto id : v->second)
      {
        SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(id);
        if (npc)
          npc->removeAtonce();
      }
      v = m_mapID2LotteryNpcID.erase(v);
      continue;
    }
    ++v;
  }
}

void ActivityEventManager::onNpcGuidChange(QWORD oldguid, QWORD id)
{
  if (m_mapID2NpcID.empty())
    return;
  for (auto& v : m_mapID2NpcID)
  {
    auto it = v.second.find(oldguid);
    if (it == v.second.end())
      continue;
    v.second.erase(oldguid);
    v.second.insert(id);
    return;
  }
}

DWORD ActivityEventManager::getExtraTimes(EAERewardMode eMode)
{
  auto it = m_mapType2Event.find(EACTIVITYEVENTTYPE_REWARD);
  if (it == m_mapType2Event.end())
    return 0;

  for (auto &m : it->second)
  {
    if (m.second.rewardinfo().mode() == eMode && m.second.rewardinfo().extratimes() != 0)
      return m.second.rewardinfo().extratimes();
  }
  return 0;
}

DWORD ActivityEventManager::getNeedResetTimes(EAERewardMode eMode)
{
  auto it = m_mapType2Event.find(EACTIVITYEVENTTYPE_RESETTIME);
  if (it == m_mapType2Event.end())
    return 0;
  DWORD cnt = 0;
  for (auto &m : it->second)
  {
    if (m.second.resetinfo().mode() == eMode && m.second.resetinfo().times() != 0)
    {
      cnt = m.second.resetinfo().times();
      break;
    }
  }
  if (cnt <= 1)
    return 0;

  switch(eMode)
  {
    case EAEREWARDMODE_MIN:
    case EAEREWARDMODE_GUILD_DONATE:
      break;
    case EAEREWARDMODE_TOWER:
    case EAEREWARDMODE_LABORATORY:
    case EAEREWARDMODE_SEAL:
    case EAEREWARDMODE_PVECARD:
      {
        DWORD cur = now();
        DWORD weekstart = xTime::getWeekStart(cur) + 5 * 3600;
        /*----------|(time1)---------|(time2)---------*/ /*ex: 3次, timetick size = 2,*/
        for (DWORD i = cnt; i > 1; --i)
        {
          DWORD resettime = weekstart + WEEK_T/cnt * (i-1);
          if (cur >= resettime)
            return i;
        }
      }
      break;
    case EAEREWARDMODE_WANTEDQUEST:
      break;
    default:
      break;
  }

  return 0;
}

DWORD ActivityEventManager::getNextResetTime(EAERewardMode eMode)
{
  DWORD cur = now();

  auto it = m_mapType2Event.find(EACTIVITYEVENTTYPE_RESETTIME);
  if (it == m_mapType2Event.end())
    return 0;
  DWORD cnt = 0;
  for (auto &m : it->second)
  {
    if (m.second.resetinfo().mode() == eMode && m.second.resetinfo().times() != 0)
    {
      cnt = m.second.resetinfo().times();
      break;
    }
  }

  switch(eMode)
  {
    case EAEREWARDMODE_MIN:
    case EAEREWARDMODE_WANTEDQUEST:
    case EAEREWARDMODE_GUILD_DONATE:
      break;
    case EAEREWARDMODE_SEAL:
    case EAEREWARDMODE_LABORATORY:
    case EAEREWARDMODE_TOWER:
    case EAEREWARDMODE_PVECARD:
      {
        DWORD weekstart = xTime::getWeekStart(cur) + 5 * 3600;
        if (cnt == 0)
          return weekstart + WEEK_T;

        for (DWORD i = 1; i <= cnt; ++i)
        {
          DWORD resettime = weekstart + WEEK_T/cnt * i;
          if (resettime >= cur)
            return resettime;
        }
      }
      break;
    default:
      break;
  }
  return cur;
}

//扭蛋购买折扣
ActivityEventInfo* ActivityEventManager::getLotteryDiscount(SceneUser* user, ELotteryType lotteryType, ECoinType coinType, DWORD dwYearMonth, DWORD count)
{
  if (!user)
    return nullptr;

  auto it = m_mapType2Event.find(EACTIVITYEVENTTYPE_LOTTERY_DISCOUNT);
  if (it == m_mapType2Event.end())
    return nullptr;
  
  if (it->second.empty())
    return nullptr;
  
  //id最大的那个才生效 
  ActivityEventInfo* pEventInfo = nullptr;

  for (auto& m : it->second)
  {
    if (!isEventStart(m.second))
      continue;
    const AELotteryDiscount& rLotteryDiscount = m.second.lotterydiscount();
    if (rLotteryDiscount.lotterytype() != lotteryType)
      continue;
    if (rLotteryDiscount.cointype() != coinType)
      continue;
    if (rLotteryDiscount.yearmonth() && rLotteryDiscount.yearmonth() != dwYearMonth)
      continue;
    pEventInfo = &m.second;
  }

  return pEventInfo;

  if (!pEventInfo)
    return nullptr;

  //DWORD userCnt = user->getUserSceneData().getActivityEventCnt(EACTIVITYEVENTTYPE_LOTTERY_DISCOUNT, pEventInfo->id(), pEventInfo->lotterydiscount().usertype());
  //XLOG << "[活动模板-扭蛋打折] charid" << user->id << user->name << "扭蛋类型" << lotteryType << "货币类型" << coinType << "使用折扣次数" << userCnt << "上限折扣次数" << pEventInfo->lotterydiscount().count() << "本次想要次数" << count << "配置折扣" << pEventInfo->lotterydiscount().discount() << XEND;
  //if ((userCnt + count) >= pEventInfo->lotterydiscount().count())
  //{
  //  return 0;
  //}
  //return pEventInfo->lotterydiscount().discount();
}

//是否是只有一个生效
bool ActivityEventManager::isOnlyOne(EActivityEventType type)
{ 
  return false;
}

void ActivityEventManager::sendActivityEventCount(SceneUser* pUser, EActivityEventType type)
{
  if (!pUser)
    return;
  
  auto it = m_mapType2Event.find(type);
  if (it == m_mapType2Event.end())
    return;

  if (it->second.empty())
    return;

  ActivityEventInfo* pEventInfo = nullptr;
  ActivityEventNtfEventCntCmd cmd;
  for (auto& m : it->second)
  {
    if (!isEventStart(m.second))
      continue;

    pEventInfo = &m.second;
    DWORD userCnt = pUser->getUserSceneData().getActivityEventCnt(type, pEventInfo->id(), pEventInfo->lotterydiscount().usertype());
    ActivityEventCnt* pCnt = cmd.add_cnt();
    if (pCnt)
    {
      pCnt->set_type(it->first);
      pCnt->set_id(m.first);
      pCnt->set_count(userCnt);
    }
  }
  
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);  
  XLOG <<"[活动模板-推送次数给前端]" <<pUser->id <<pUser->name <<"msg"<<cmd.ShortDebugString() << XEND;
}

bool ActivityEventManager::isEventStop(QWORD id, TMapId2ActivityEventInfo& rMapEvent)
{
  auto it = rMapEvent.find(id);
  if (it == rMapEvent.end())
    return true;
  
  if (isEventStart(it->second))
    return false;
  return true;
}

DWORD ActivityEventManager::getLotteryNpcId(Cmd::ELotteryType lotteryType)
{
  //static std::map<ELotteryType, DWORD> mapType2Id = {
  //  { ELotteryType_Head, 1695 },
  //  { ELotteryType_Equip, 1696 },
  //  { ELotteryType_Card, 1697 },
  //  { ELotteryType_Magic, 5106 } };  
  switch (lotteryType)
  {
  case ELotteryType_Head:
    return 1695;
  case ELotteryType_Equip:
    return 1696;
  case ELotteryType_Card:
    return 1697;
  case ELotteryType_Magic:
    return 5106;
  default:
    break;
  }
  return 0;
}

DWORD ActivityEventManager::getGuildBuildingSubmitInc(EGuildBuilding type, DWORD level, EGBuildingSubmitInc inc)
{
  auto it = m_mapType2Event.find(EACTIVITYEVENTTYPE_GUILD_BUILDING_SUBMIT);
  if (it == m_mapType2Event.end())
    return 0;

  for (auto& v : it->second)
  {
    if (isEventStart(v.second) == false)
      continue;
    const AEGuildBuildingSubmitInfo& info = v.second.gbuildingsubmitinfo();
    bool f = false;
    for (int i = 0; i < info.types_size(); ++i)
      if (info.types(i) == type)
      {
        f = true;
        break;
      }
    if (f && info.minlv() <= level && info.maxlv() >= level)
    {
      switch (inc)
      {
      case EGBUILDINGSUBMITINC_SUBMIT:
        return info.submitinc();
      case EGBUILDINGSUBMITINC_REWARD:
        return info.rewardinc();
      default:
        return 0;
      }
    }
  }
  return 0;
}

void ActivityEventManager::updateShopCache()
{
  m_mapID2ShopItem.clear();

  auto it = m_mapType2Event.find(EACTIVITYEVENTTYPE_SHOP);
  if (it == m_mapType2Event.end())
    return;

  for (auto& v : it->second)
  {
    for (int i = 0; i < v.second.shopinfo().items_size(); ++i)
      m_mapID2ShopItem[v.second.shopinfo().items(i).id()][v.second.begintime()] = v.second.shopinfo().items(i);
  }
  return;
}

// 商店活动只有开始时间, 没有结束时间, 相当于一个定时开启的配置
const ShopItem* ActivityEventManager::getShopItem(DWORD shopid)
{
  auto itact = m_mapType2Event.find(EACTIVITYEVENTTYPE_SHOP);
  if (itact == m_mapType2Event.end())
    return nullptr;

  auto it = m_mapID2ShopItem.find(shopid);
  if (it == m_mapID2ShopItem.end())
    return nullptr;
  const ShopItem* item = nullptr;
  DWORD cur = now();
  for (auto& v : it->second)
  {
    if (cur > v.first)
      item = &v.second;
    else
      break;
  }
  return item;
}

bool ActivityEventManager::canBuyShopItem(DWORD shopid)
{
  const ShopItem* item = getShopItem(shopid);
  if (!item || (!item->adddate() && !item->removedate()))
    return true;
  DWORD cur = now();
  return cur >= item->adddate() && cur < item->removedate();
}

DWORD ActivityEventManager::getShopItemOrder(DWORD shopid)
{
  const ShopItem* item = getShopItem(shopid);
  if (!item || !item->shoporder())
    return 0;
  return item->shoporder();
}
