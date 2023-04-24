#include "GuildRaidConfig.h"
#include "xLuaTable.h"
#include "MiscConfig.h"

GuildRaidConfig::GuildRaidConfig()
{

}

GuildRaidConfig::~GuildRaidConfig()
{

}

bool GuildRaidConfig::loadConfig()
{
  bool bCorrect = true;
  if (loadMapConfig() == false)
    bCorrect = false;
  if (loadMonsterConfig() == false)
    bCorrect = false;
  if (loadGuildCityConfig() == false)
    bCorrect = false;
  return bCorrect;
};

bool GuildRaidConfig::loadMapConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_GuildPVE_Map.txt")) {
    XERR << "[公会副本配置] 加载配置Table_GuildPVE_Map.txt失败" << XEND;
    return false;
  }

  bool bCorrect = false;
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_GuildPVE_Map", table);
  DWORD maxPassLinkNode = 0;

  m_mapGuildRaid.clear();
  m_mapType2GuildRaids.clear();

  map<EGuildRaidMapType, map<bool,set<DWORD>>> linkNums;

  for (auto m = table.begin(); m != table.end(); ++m) {
    SGuildRaidMap rMap;
    rMap.dwMapID = m->first;
    rMap.eMapType = static_cast<EGuildRaidMapType>(m->second.getTableInt("Type"));
    rMap.haveBoss = m->second.getTableInt("HaveBoss");

    xLuaData ids = m->second.getMutableData("LinkMapType");
    auto func = [&](const string & key, xLuaData& data) {
      rMap.setExitIDs.insert(data.getInt());
    };
    ids.foreach(func);

    linkNums[rMap.eMapType][rMap.haveBoss].insert(rMap.setExitIDs.size());

    // check
    switch (rMap.eMapType) {
    case EGUILDRAIDMAP_ENTRANCE:
    {
      if (rMap.setExitIDs.size() != 1) {
        XERR << "[公会副本配置] 入口节点id:" << rMap.dwMapID << "link数不等于1" << XEND;
        bCorrect = false;
        continue;
      }
      if (rMap.haveBoss) {
        XERR << "[公会副本配置] 入口节点id:" << rMap.dwMapID << "不能有boss" << XEND;
        bCorrect = false;
        continue;
      }
      break;
    }
    case EGUILDRAIDMAP_PASS:
    {
      if (rMap.setExitIDs.size() < 2) {
        XERR << "[公会副本配置] 过道节点id:" << rMap.dwMapID << "link数小于2" << XEND;
        bCorrect = false;
        continue;
      }

      if (rMap.setExitIDs.size() > maxPassLinkNode)
        maxPassLinkNode = rMap.setExitIDs.size();
      break;
    }
    case EGUILDRAIDMAP_NODE:
    {
      if (rMap.setExitIDs.size() != 1) {
        XERR << "[公会副本配置] 断路节点id:" << rMap.dwMapID << "link数不等于1" << XEND;
        bCorrect = false;
        continue;
      }
      break;
    }
    case EGUILDRAIDMAP_END:
    {
      if (rMap.setExitIDs.size() != 1) {
        XERR << "[公会副本配置] 终点节点id:" << rMap.dwMapID << "link数不等于1" << XEND;
        bCorrect = false;
        continue;
      }
      if (!rMap.haveBoss) {
        XERR << "[公会副本配置] 终点节点id:" << rMap.dwMapID << "必须有boss" << XEND;
        bCorrect = false;
        continue;
      }
      break;
    }
    default:
      XERR << "[公会副本配置] 节点id:" << rMap.dwMapID << "type非法" << XEND;
      bCorrect = false;
      continue;
    }

    m_mapGuildRaidMap[rMap.dwMapID] = rMap;
    m_mapType2GuildRaids[rMap.eMapType].push_back(rMap.dwMapID);
  }

  for (auto it = linkNums.begin(); it != linkNums.end(); ++it) {
    switch (it->first) {
    case EGUILDRAIDMAP_PASS:
    case EGUILDRAIDMAP_NODE:
      if (it->second[true].size() != it->second[false].size()) {
        XERR << "[公会副本配置] 节点type:" << it->first << "有boss和没boss的传送点数不同" << XEND;
        bCorrect = false;
        continue;
      }
      if (it->first == EGUILDRAIDMAP_PASS && it->second[true].size() != maxPassLinkNode - 1) {
        XERR << "[公会副本配置] 过道节点传送点数未覆盖全部情况" << XEND;
        bCorrect = false;
        continue;
      }
      break;
    default:
      break;
    }
  }

  m_dwMaxPassLinkNode = maxPassLinkNode;
  if (bCorrect)
    XLOG << "[公会副本配置] 加载配置Table_GuildPVE_Map.txt 成功" << XEND;

  return bCorrect;
}

bool GuildRaidConfig::checkConfig()
{
  return true;
}

bool GuildRaidConfig::createGuildRaid(bool bIgnoreTime /*= false*/)
{
  FUN_TIMECHECK_30();
  DWORD time = now();
  if (bIgnoreTime == false)
  {
    if (time < m_dwNextCreatTime)
      return true;
  }
  DWORD weektime = xTime::getWeekStart(time, 3600 * 5);
  m_dwNextCreatTime = weektime + 86400 * 7;

  DWORD srandtime = getSRandTime();
  srand(srandtime);

  // total:总数, max:每个位置分配的最大值, pos:总共分配位置数
  auto randFunc = [](DWORD total, DWORD max, DWORD pos, TVecDWORD& out) -> bool {
    if (pos <= 0 || pos * max < total)
      return false;

    for (DWORD i = 0; i < pos; ++i)
      out.push_back(0);

    DWORD tail = pos - 1;
    for (DWORD i = 0; i < total; ++i) {
      if (tail == 0) {
        out[0] += total - i;
        break;
      }
      DWORD idx = randBetween(0, tail);
      ++out[idx];
      if (out[idx] >= max) {
        swap(out[idx], out[tail]);
        --tail;
      }
    }

    random_shuffle(out.begin(), out.end());

    return true;
  };

  m_mapGuildRaid.clear();

  const SGuildRaidCFG& rCFG = MiscConfig::getMe().getGuildRaidCFG();
  DWORD maxsize = rCFG.getMapGroupSize();
  if (maxsize == 0)
  {
    XERR << "[公会副本], 创建失败, 组数配置为0" << XEND;
    return false;
  }

  for (DWORD i = 1; i <= maxsize; ++i)
  {
    // 终点必有boss
    DWORD bossNum = rCFG.dwBossMapNum - 1;
    if (bossNum < 1) {
      XERR << "[公会副本-随机boss数] 配置错误, dwBossMapNum:" << rCFG.dwBossMapNum << XEND;
      bossNum = 5;
    }

    // 深度
    // 深度最少为boss数+1
    DWORD mapDepth = randBetween(rCFG.pairMapDepth.first, rCFG.pairMapDepth.second);
    if (mapDepth < bossNum + 1) {
      XERR << "[公会副本-随机深度] 配置错误, mapDepth:" << mapDepth << "bossNum:"
           << bossNum << "最小:" << rCFG.pairMapDepth.first << "最大:" << rCFG.pairMapDepth.second << XEND;
      mapDepth = bossNum + 1;
    }

    // 节点数
    // 最小节点数: 深度
    // 最大节点数: (深度 - 2) * (pass节点连接最大node数 - 2) + 深度
    DWORD minNum = mapDepth;
    DWORD maxNum = (mapDepth - 2) * (m_dwMaxPassLinkNode - 2) + mapDepth;
    if (rCFG.pairMapNumRange.first < minNum || rCFG.pairMapNumRange.second > maxNum) {
      minNum = rCFG.pairMapNumRange.first < minNum ? minNum : (rCFG.pairMapNumRange.first > maxNum ? maxNum : rCFG.pairMapNumRange.first);
      maxNum = rCFG.pairMapNumRange.second > maxNum ? maxNum : rCFG.pairMapNumRange.second;
    } else {
      minNum = rCFG.pairMapNumRange.first;
      maxNum = rCFG.pairMapNumRange.second;
    }
    DWORD totalNum = randBetween(minNum, maxNum);

    TVecDWORD nodeNums, haveBosss;
    // 为pass节点随机node数
    if (!randFunc(totalNum - mapDepth, m_dwMaxPassLinkNode - 2, mapDepth - 2, nodeNums)) {
      XERR << "[公会副本-随机node数] 错误, totalNum:" << totalNum << "mapDepth:" << mapDepth << XEND;
      return false;
    }
    // 随机boss分布
    if (!randFunc(bossNum, 1, totalNum - 2, haveBosss)) {
      XERR << "[公会副本-随机boss分布] 错误, bossNum:" << bossNum << "totalNum:" << totalNum << XEND;
      return false;
    }

    // 创建
    DWORD idx = 0;
    if (createGuildRaidRoute(i, idx, nodeNums, haveBosss, mapDepth, EGUILDRAIDMAP_ENTRANCE, nullptr, nullptr) == false)
      return false;
  }

  // print result
  XLOG << "[公会副本], 创建开始:" << XEND;
  for (auto &m : m_mapGuildRaid)
  {
    XLOG << "[公会副本], 当前组号:" << m.first << "创建信息如下: "<< XEND;
    for (auto &it : m.second)
    {
      //std::cout << "index is :" << it.second.dwIndex << "   map is:" << it.second.dwMapID << std::endl;
      XLOG << "[公会副本]"<< "index is :" << it.second.dwIndex << "   map is:" << it.second.dwMapID << XEND;
      for (auto &k : it.second.mapExit2Link)
      {
        //std::cout << "link info:" << k.second.dwNextIndex << "   map :" << k.second.dwNextMapID << std::endl;
        XLOG << "[公会副本]" << "link info:" << k.second.dwNextIndex << "   map :" << k.second.dwNextMapID << XEND;
      }
      //std::cout << "-------------" << std::endl;
    }
  }
  XLOG << "[公会副本], 创建结束:" << XEND;
  // 重置随机数种子
  srand(xTime::getCurUSec());

  return true;
}

bool GuildRaidConfig::createGuildRaidRoute(DWORD group, DWORD& idx, TVecDWORD& nodeNums, TVecDWORD& haveBosss, DWORD depth, EGuildRaidMapType type, SGuildRaidInfo* preInfo, SGuildRaidLink* preLink)
{
  if (depth < 1 || type <= EGUILDRAIDMAP_MIN || type > EGUILDRAIDMAP_END) {
    XERR << "[公会副本-创建副本地图] 创建错误, depth:" << depth << "type:" << type << XEND;
    return false;
  }

  // 计算节点link数
  DWORD curLinkNum = 0;
  if (type != EGUILDRAIDMAP_PASS)
    curLinkNum = 1;
  else {
    curLinkNum = 2 + nodeNums.back();
    nodeNums.pop_back();
  }

  // 是否有boss
  bool haveBoss = type == EGUILDRAIDMAP_END ? true : false;
  if (type == EGUILDRAIDMAP_PASS || type == EGUILDRAIDMAP_NODE) {
    haveBoss = haveBosss.back();
    haveBosss.pop_back();
  }

  const SGuildRaidMap* randMap = getRandGuildRaidMapByType(type, haveBoss, curLinkNum);
  if (randMap == nullptr) {
    XERR << "[公会副本-创建副本地图] 创建错误, type:" << type << "" << "haveBoss:" << haveBoss << "curLinkNum:" << curLinkNum << "找不到地图信息" << XEND;
    return false;
  }

  // 构造节点
  SGuildRaidInfo curInfo;
  curInfo.dwIndex = ++idx;
  curInfo.dwMapID = randMap->dwMapID;
  curInfo.dwGroupID = group;

  // 打乱传送点
  TVecDWORD randExitIDs;
  randMap->getRandomExitIDs(randExitIDs);

  // 构造传送点
  for (size_t i = 0; i < randExitIDs.size(); ++i) {
    SGuildRaidLink curLink;
    curLink.dwMyExitID = randExitIDs[i];
    curLink.dwGroup = group;

    // 第一个link总是用于连接上一个节点(entrance除外)
    if (i == 0 && preInfo != nullptr && preLink != nullptr) {
      preLink->dwNextIndex = curInfo.dwIndex;
      preLink->dwNextMapID = curInfo.dwMapID;
      preLink->dwNextBornPoint = curLink.dwMyExitID;
      curLink.dwNextIndex = preInfo->dwIndex;
      curLink.dwNextMapID = preInfo->dwMapID;
      curLink.dwNextBornPoint = preLink->dwMyExitID;
    }

    EGuildRaidMapType nextType = EGUILDRAIDMAP_MIN;
    DWORD nextDepth = depth;

    switch (type) {
    case EGUILDRAIDMAP_ENTRANCE:
    {
      if (i == 0) {             // entrance节点的第一个link用于连接下一个pass节点
        if (depth <= 1) {
          XERR << "[公会副本-创建副本地图] 创建错误, depth不合法" << XEND;
          return false;
        }
        nextDepth = depth - 1;
        nextType = nextDepth == 1 ? EGUILDRAIDMAP_END : EGUILDRAIDMAP_PASS;
      } else {
        XERR << "[公会副本-创建副本地图] 创建错误, 入口节点只有1个link" << XEND;
        return false;
      }
      break;
    }
    case EGUILDRAIDMAP_PASS:
    {
      if (i == 1) {              // pass节点的第二个link用于连接下一个pass/end
        if (depth <= 1) {
          XERR << "[公会副本-创建副本地图] 创建错误, depth不合法" << XEND;
          return false;
        }
        nextDepth = depth - 1;
        nextType = nextDepth == 1 ? EGUILDRAIDMAP_END : EGUILDRAIDMAP_PASS;
      } else if (i > 1) {       // pass节点的第三个及后续link用于连接node节点
        nextDepth = depth;
        nextType = EGUILDRAIDMAP_NODE;
      }
      break;
    }
    case EGUILDRAIDMAP_NODE:
    case EGUILDRAIDMAP_END:
      if (i > 0) {
        XERR << "[公会副本-创建副本地图] 创建错误type:" << type << "节点只能有1个link" << XEND;
        return false;
      }
      break;
    default:
      XERR << "[公会副本-创建副本地图] 创建错误, type不合法" << XEND;
      return false;
    }

    if (nextType != EGUILDRAIDMAP_MIN && !createGuildRaidRoute(group, idx, nodeNums, haveBosss, nextDepth, nextType, &curInfo, &curLink))
      return false;

    curInfo.mapExit2Link[curLink.dwMyExitID] = curLink;
  }

  m_mapGuildRaid[group][curInfo.dwIndex] = curInfo;

  return true;
}

void SGuildRaidMap::getRandomExitIDs(TVecDWORD& vecIDs) const
{
  vecIDs.clear();
  for (auto id : setExitIDs) {
    vecIDs.push_back(id);
  }
  random_shuffle(vecIDs.begin(), vecIDs.end());
}

const SGuildRaidMap* GuildRaidConfig::getGuildRaidMap(DWORD id) const
{
  auto v = m_mapGuildRaidMap.find(id);
  if (v == m_mapGuildRaidMap.end())
    return nullptr;
  return &v->second;
}

const SGuildRaidMap* GuildRaidConfig::getRandGuildRaidMapByType(EGuildRaidMapType type, bool haveBoss, DWORD linkNum) const
{
  auto it = m_mapType2GuildRaids.find(type);
  if (it == m_mapType2GuildRaids.end() || it->second.size() == 0) {
    return nullptr;
  }

  TVecDWORD mapIDs;
  for (auto id : it->second) {
    const SGuildRaidMap* rMap = getGuildRaidMap(id);
    if (rMap != nullptr && rMap->haveBoss == haveBoss && rMap->setExitIDs.size() == linkNum)
      mapIDs.push_back(id);
  }
  if (mapIDs.size() <= 0)
    return nullptr;

  return getGuildRaidMap(mapIDs[randBetween(0, mapIDs.size()-1)]);
}

const SGuildRaidInfo* GuildRaidConfig::getGuildRaidInfo(DWORD dwRaidIndex) const
{
  DWORD dwGroup = dwRaidIndex / ONE_THOUSAND;
  auto it = m_mapGuildRaid.find(dwGroup);
  if (it == m_mapGuildRaid.end())
    return nullptr;
  DWORD dwIndex= dwRaidIndex - dwGroup * ONE_THOUSAND;
  auto m = it->second.find(dwIndex);
  if (m == it->second.end())
    return nullptr;

  return &(m->second);
}

const SGuildRaidInfo* GuildRaidConfig::getGuildEntrance(DWORD dwRaidGroup) const
{
  auto it = m_mapGuildRaid.find(dwRaidGroup);
  if (it == m_mapGuildRaid.end())
    return nullptr;
  auto m = it->second.find(1);
  if (m == it->second.end())
    return nullptr;

  return &(m->second);
}


// raid monster
bool GuildRaidConfig::loadMonsterConfig()
{
  bool bCorrect = true;

  if (!xLuaTable::getMe().open("Lua/Table/Table_GuildPVE_Monster.txt"))
  {
    XERR << "[公会副本配置] 加载配置Table_GuildPVE_Map.txt失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_GuildPVE_Monster", table);

  m_mapGuildRaidMonster.clear();
  m_mapLv2RaidMonster.clear();

  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SGuildRaidMonster rCFG;
    rCFG.dwNpcID = m->first;
    rCFG.dwUniqID = m->second.getTableInt("GroupID");
    rCFG.dwWeight = m->second.getTableInt("Weight");
    if (rCFG.dwUniqID == 0 || rCFG.dwWeight == 0)
    {
      XERR << "[公会副本], Table_GuildPVE_Monster配置错误, id:" << m->first << "GroupID和Wight不可为0" << XEND;
      bCorrect = false;
    }
    rCFG.dwTeamGroup = m->second.getTableInt("TeamID");
    auto getrange = [&](const string& key, xLuaData& data)
    {
      rCFG.setLvs.insert(data.getInt());
    };
    m->second.getMutableData("LevelRange").foreach(getrange);

    auto getrwd = [&](const string& key, xLuaData& data)
    {
      rCFG.setBossReward.insert(data.getInt());
    };
    m->second.getMutableData("BossReward").foreach(getrwd);

    m_mapGuildRaidMonster[m->first] = rCFG;

    if (rCFG.dwTeamGroup != 0)
      m_mapGroup2Mosnter[rCFG.dwTeamGroup].insert(rCFG.dwNpcID);

    for (auto &s : rCFG.setLvs)
    {
      TMapUniqID2RaidMonser& mapUidMon = m_mapLv2RaidMonster[s];
      TVecGuildRaidMonster& vecMon = mapUidMon[rCFG.dwUniqID];
      if (vecMon.empty())
      {
        vecMon.push_back(rCFG);
      }
      else
      {
        // 记录weight
        DWORD oriWight = rCFG.dwWeight;

        rCFG.dwWeight += vecMon.back().dwWeight;
        vecMon.push_back(rCFG);
        // 重置wight
        rCFG.dwWeight = oriWight;
      }
    }
  }
  if (bCorrect)
    XLOG << "[公会副本配置] 加载配置Table_GuildPVE_Monster.txt 成功" << XEND;
  return bCorrect;
}

bool GuildRaidConfig::getRandomMonster(DWORD lv, DWORD uid, TSetDWORD& setNpcs) const
{
  auto m = m_mapLv2RaidMonster.find(lv);
  if (m == m_mapLv2RaidMonster.end())
    return false;
  auto it = m->second.find(uid);
  if (it == m->second.end() || it->second.empty())
    return false;

  DWORD totalweight = it->second.back().dwWeight;
  DWORD rand = randBetween(1, totalweight);
  DWORD teamid = 0;
  for (auto &v : it->second)
  {
    if (v.dwWeight >= rand)
    {
      teamid = v.dwTeamGroup;
      setNpcs.insert(v.dwNpcID);
      break;
    }
  }
  if (teamid)
  {
    auto k = m_mapGroup2Mosnter.find(teamid);
    if (k != m_mapGroup2Mosnter.end())
    {
      setNpcs.insert(k->second.begin(), k->second.end());
    }
  }

  return true;
}

bool GuildRaidConfig::getRandomMonster(DWORD lv, const TSetDWORD& uids, TSetDWORD& setNpcs, DWORD srandGuildID /*=0*/) const
{
  DWORD randuid = 0;
  if (uids.size() == 0)
    return false;

  /* -- notice
    srandGuildID公会ID, 传入 != 0时, 表示需要公会随机招怪一致
  */
  if (srandGuildID)
  {
    DWORD srandtime = getSRandTime();
    srand(srandtime + srandGuildID);
  }

  if (uids.size() == 1)
  {
    randuid = *(uids.begin());
  }
  else
  {
    auto m = m_mapLv2RaidMonster.find(lv);
    if (m != m_mapLv2RaidMonster.end())
    {
      // 根据不同uid的总权重比例,先随机一个uid
      map<DWORD, DWORD> uid2rate;
      DWORD tmpCurRate = 0;
      for (auto &s : uids)
      {
        auto it = m->second.find(s);
        if (it == m->second.end() || it->second.empty())
          continue;
        tmpCurRate += it->second.back().dwWeight;
        uid2rate[s] = tmpCurRate;
      }
      DWORD rand = randBetween(1, tmpCurRate);
      for (auto &u : uid2rate)
      {
        if (u.second >= rand)
        {
          randuid = u.first;
          break;
        }
      }
    }
  }

  bool randok = getRandomMonster(lv, randuid, setNpcs);

  // 重置随机数种子
  if (srandGuildID)
    srand(xTime::getCurUSec());

  return randok;
}

bool GuildRaidConfig::isBossMonster(DWORD npcid) const
{
  auto it = m_mapGuildRaidMonster.find(npcid);
  if (it == m_mapGuildRaidMonster.end())
    return false;
  return !it->second.setBossReward.empty();
}

const TSetDWORD& GuildRaidConfig::getBossReward(DWORD npcid) const
{
  auto it = m_mapGuildRaidMonster.find(npcid);
  if (it == m_mapGuildRaidMonster.end())
  {
    static const TSetDWORD emptyset;
    return emptyset;
  }
  return it->second.setBossReward;
}

bool GuildRaidConfig::resetRaid(DWORD timeversion)
{
  if (m_dwResetTimeVersion >= timeversion)
  {
    XERR << "[公会副本-外部重置], 当前版本号:" << m_dwResetTimeVersion << "已大于版本号:" << timeversion << XEND;
    return false;
  }

  m_dwResetTimeVersion = timeversion;
  XLOG << "[公会副本-外部重置], 重置版本号:" << timeversion << XEND;

  return createGuildRaid(timeversion);
}

DWORD GuildRaidConfig::getSRandTime() const
{
  // 有指定重置时间则优先, 否则以本周周一05:00:00为重置时间戳
  return m_dwResetTimeVersion ? m_dwResetTimeVersion : xTime::getWeekStart(now(), 3600 * 5);
}

/********************公会战相关***********************/
bool GuildRaidConfig::loadGuildCityConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_Guild_StrongHold.txt"))
  {
    XERR << "[公会据点配置] 加载配置Table_Guild_StrongHold.txt失败" << XEND;
    return false;
  }

  bool bCorrect = true;
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Guild_StrongHold", table);

  m_mapGuildCityCFG.clear();
  for (auto &it : table)
  {
    auto m = m_mapGuildCityCFG.find(it.first);
    if (m != m_mapGuildCityCFG.end())
    {
      XERR << "[Table_Guild_StrongHold], ID配置重复, ID:" << it.first << XEND;
      bCorrect = false;
      continue;
    }
    SGuildCityCFG& rCFG = m_mapGuildCityCFG[it.first];
    rCFG.dwCityID = it.first;
    rCFG.dwMapID = it.second.getTableInt("MapId");
    rCFG.dwRaidID = it.second.getTableInt("RaidId");
    //rCFG.dwEpID = it.second.getTableInt("EntryTP");

    xLuaData& tele = it.second.getMutableData("TelePort");
    rCFG.dwTeleMapID = tele.getTableInt("1");
    rCFG.dwTeleBpID = tele.getTableInt("2");

    xLuaData& flag = it.second.getMutableData("Flag");
    rCFG.oFlag.x = flag.getTableFloat("1");
    rCFG.oFlag.y = flag.getTableFloat("2");
    rCFG.oFlag.z = flag.getTableFloat("3");

    rCFG.strName = it.second.getTableString("Name");

    auto grpid = [&](const string& k, xLuaData& d)
    {
      rCFG.setGroupRaids.insert(d.getInt());
    };
    it.second.getMutableData("Gvg_Group").foreach(grpid);

    rCFG.bOpen = it.second.getTableInt("Open") == 1;
    rCFG.bSuper = it.second.getTableInt("Super") == 1;

    xLuaData& safe = it.second.getMutableData("SafeArea");
    auto getmappos = [&](const string& k, xLuaData& d)
    {
      DWORD mapid = d.getTableInt("map");
      if (mapid == 0)
        return;
      vector<xPos>& posvec = rCFG.mapSafeMapID2Pos[mapid];

      auto getpos = [&](xLuaData& posd)
      {
        xPos p;
        p.x = posd.getTableFloat("1");
        p.y = posd.getTableFloat("2");
        p.z = posd.getTableFloat("3");
        if (p.empty())
          return;
        posvec.push_back(p);
      };
      getpos(d.getMutableData("p1"));
      getpos(d.getMutableData("p2"));
      getpos(d.getMutableData("p3"));
      getpos(d.getMutableData("p4"));

      if (posvec.size() != 4)
      {
        XERR << "[Guild_StrongHold],配置错误, 安全区坐标配置错误, id:" << it.first << XEND;
        bCorrect = false;
      }
    };
    safe.foreach(getmappos);

    xLuaData& limitep = it.second.getMutableData("OnlyDefExitPoints");
    auto getlimitep = [&](const string& k, xLuaData& d)
    {
      DWORD mapid = atoi(k.c_str());
      TSetDWORD& epids = rCFG.mapLimitMapID2Ep[mapid];
      d.getIDList(epids);
    };
    limitep.foreach(getlimitep);

    xLuaData& treasure = it.second.getMutableData("GuildReward");
    auto treasuref = [&](const string& key, xLuaData& data)
    {
      DWORD dwTime = atoi(key.c_str());
      TMapTreasure& mapCFG = rCFG.mapTreasure[dwTime];

      auto countf = [&](const string& key, xLuaData& d)
      {
        DWORD& rCount = mapCFG[d.getTableInt("1")];
        rCount += d.getTableInt("2");
      };
      data.foreach(countf);
    };
    treasure.foreach(treasuref);

    for (auto &m : rCFG.mapTreasure)
    {
      for (auto &count : m.second)
        XDBG << "[Table_Guild_StrongHold]" << rCFG.dwCityID << "treasure count :" << m.first << "id :" << count.first << count.second << XEND;
    }

    m_mapID2CityID[rCFG.dwMapID].insert(it.first);
  }

  if (bCorrect)
    XLOG << "[Table_Guild_StrongHold], 公会据点配置加载成功" << XEND;
  return true;
}

const SGuildCityCFG* GuildRaidConfig::getGuildCityCFG(DWORD cityid) const
{
  auto it = m_mapGuildCityCFG.find(cityid);
  if (it == m_mapGuildCityCFG.end())
    return nullptr;
  return &(it->second);
}

/*const SGuildCityCFG* GuildRaidConfig::getGuildCityCFGByEP(DWORD mapid, DWORD epid) const
{
  for (auto &m : m_mapGuildCityCFG)
  {
    if (m.second.dwMapID == mapid && m.second.dwEpID == epid)
      return &(m.second);
  }
  return nullptr;
}
*/
const SGuildCityCFG* GuildRaidConfig::getGuildCityCFGByRaid(DWORD raidid) const
{
  for (auto &m : m_mapGuildCityCFG)
  {
    if (m.second.dwRaidID == raidid)
      return &(m.second);
  }
  return nullptr;
}

bool GuildRaidConfig::collectCityID(DWORD dwMapID, TSetDWORD& setIDs) const
{
  setIDs.clear();

  auto m = m_mapID2CityID.find(dwMapID);
  if (m == m_mapID2CityID.end())
    return false;

  setIDs = m->second;
  return true;
}

bool SGuildCityCFG::getRecPosAndWidth(DWORD mapid, xPos& p1, xPos& p2, float& w) const
{
  auto it = mapSafeMapID2Pos.find(mapid);
  if (it == mapSafeMapID2Pos.end())
    return false;

  if (it->second.size() != 4)
    return false;

  /*
   * (1)----------(4)
   *  .            .
   *  p1           p2 (w)
   *  .            .
   * (2)----------(3)
   * */
  const xPos& pos1 = it->second[0];
  const xPos& pos2 = it->second[1];
  const xPos& pos3 = it->second[2];
  const xPos& pos4 = it->second[3];

  p1.x = (pos1.x + pos2.x) / 2.0f;
  p1.y = (pos1.y + pos2.y) / 2.0f;
  p1.z = (pos1.z + pos2.z) / 2.0f;

  p2.x = (pos3.x + pos4.x) / 2.0f;
  p2.y = (pos3.y + pos4.y) / 2.0f;
  p2.z = (pos3.z + pos4.z) / 2.0f;

  w = (getDistance(pos1, pos2) + getDistance(pos3, pos4)) / 2.0f;

  return true;
}
