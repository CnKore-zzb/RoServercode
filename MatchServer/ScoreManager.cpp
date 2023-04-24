#include "ScoreManager.h"
#include "MatchServer.h"
#include "MiscConfig.h"
#include "xDBConnPool.h"
#include "GCharManager.h"
#include "MailManager.h"

void SPwsScoreData::getDataByGChar()
{
  GCharReader gChar(thisServer->getRegionID(), qwCharID);
  // get portrait data by gchar
  gChar.getBySocial();
  strUserName = gChar.getName();
  eProfession = gChar.getProfession();
  oPortrait.set_portrait(gChar.getPortrait());
  oPortrait.set_body(gChar.getBody());
  oPortrait.set_hair(gChar.getHair());
  oPortrait.set_haircolor(gChar.getHairColor());
  oPortrait.set_gender(gChar.getGender());
  oPortrait.set_head(gChar.getHead());
  oPortrait.set_face(gChar.getFace());
  oPortrait.set_mouth(gChar.getMouth());
  oPortrait.set_eye(gChar.getEye());
}


TeamPwsRank::TeamPwsRank()
{

}

TeamPwsRank::~TeamPwsRank()
{
  if (m_bUpdateRank)
    saveRankData();
}

void TeamPwsRank::loadData()
{
  xField* field = thisServer->getDBConnPool().getField(REGION_DB, TEAM_PWS_TABLE_NAME);
  if (field == nullptr)
  {
    XERR << "[ScoreManager-加载数据], 失败, 找不到" << TEAM_PWS_TABLE_NAME << XEND;
    return;
  }

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, nullptr, nullptr);
  if (ret == QWORD_MAX)
  {
    XERR << "[ScoreManager-加载], 加载数据失败" << XEND;
    return;
  }

  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG();
  std::map<DWORD, SPwsScoreData> mapTempRankData;
  for (QWORD i = 0; i < ret; ++i)
  {
    const auto& d = set[i];
    QWORD charid = d.get<QWORD>("charid");
    DWORD score = d.get<DWORD>("score");
    DWORD rank = d.get<DWORD>("rank");

    m_mapCharID2Score[charid] = score;
    if (rank)
    {
      SPwsScoreData& data = mapTempRankData[rank];
      data.qwCharID = charid;
      data.dwScore = score;
      data.eRank = rCFG.getERankByScore(score);
      data.getDataByGChar();
    }
    else
    {
      recordOutRankUser(charid, score);
    }
  }

  for (auto &m : mapTempRankData)
  {
    if (m_listScores.size() == m_dwMaxCacheNum)
      break;
    m_listScores.push_back(m.second);
    m_setAllRankUsers.insert(m.second.qwCharID);
  }
  updateRankInfo(true);

  XLOG << "[组队排位赛-积分数据加载], 加载数据库成功, 加载数据条数:" << ret << "加载排名记录:" << mapTempRankData.size() << XEND;
}

void TeamPwsRank::updateScore(const UpdateScoreMatchSCmd& cmd)
{
  xTime frameTimer;
  for (int i = 0; i < cmd.userscores_size(); ++i)
  {
    auto &d = cmd.userscores(i);
    int change = d.score();
    if (change == 0)
      continue;

    int oldscore = getUserScore(d.charid());
    DWORD newscore = (change + oldscore > 0) ? change + oldscore : 0;

    changeScore(d.charid(), change, d);

    XLOG << "[组队排位赛-副本结束积分变化], 玩家:" << d.charid() << "旧的积分:" << oldscore << "新的积分:" << newscore << XEND;
  }

  // 保存积分数据
  saveScoreData();
  // 更新前200名查询列表
  updateRankInfo();

  QWORD e = frameTimer.uElapse();
  if (e >= 1000)
    XLOG << "[组队排位赛-副本积分更新], 耗时超过1000微妙" << e << XEND;

  XLOG << "[组队排位赛-副本积分更新], 收到更新积分消息" << cmd.ShortDebugString() << XEND;
}

void TeamPwsRank::changeScore(QWORD charid, int change)
{
  changeScore(charid, change, MatchScoreData());

  // 保存积分数据
  saveScoreData();
  // 更新前200名查询列表
  updateRankInfo();

  XLOG << "[组队排位赛-积分更改], 玩家:" << charid << "变化值:" << change << XEND;
}

void TeamPwsRank::changeScore(QWORD charid, int change, const MatchScoreData& sdata)
{
  if (change == 0)
    return;
  int oldscore = getUserScore(charid);
  DWORD newscore = oldscore + change > 0  ? oldscore + change: 0;

  auto formatData = [&](const MatchScoreData& msdata, SPwsScoreData& todata)
  {
    if (!sdata.has_portrait())
      return;

    todata.strUserName = msdata.name();
    todata.oPortrait.CopyFrom(msdata.portrait());
    todata.eProfession = msdata.profession();
  };

  if (inRankList(charid))
  {
    const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG();
    SPwsScoreData data(charid, newscore);
    data.eRank = rCFG.getERankByScore(newscore);
    formatData(sdata, data);

    updateUserInRankList(data, false);
    m_bTempRankChange = true;
    m_bUpdateRank = true;
  }
  else if (checkAdd(newscore))
  {
    const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG();
    SPwsScoreData data(charid, newscore);
    data.eRank = rCFG.getERankByScore(newscore);
    formatData(sdata, data);

    insertNewData(data);
    m_bTempRankChange = true;
    m_bUpdateRank = true;
  }
  else
  {
    removeOutRankUser(charid, oldscore);
    recordUserScore(charid, newscore);
  }

  m_mapTempDirtyScore[charid] = std::make_pair(newscore, change<0);
}

void TeamPwsRank::saveScoreData()
{
  if (m_mapTempDirtyScore.empty())
    return;

  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, TEAM_PWS_TABLE_NAME);
  if (pField == nullptr)
    return;
  for (auto &m : m_mapTempDirtyScore)
  {
    // update database
    xSQLAction* pAct = xSQLThread::create(pField);
    if (!pAct)
    {
      XERR << "[组队排位赛-积分保存], 创建xSQLAction失败, 玩家:" << m.first << "积分:" << m.second.first << XEND;
      continue;
    }
    xRecord& record = pAct->m_oRecord;
    if (m.second.second)
    {
      // 减分时, 不更新得分时间
      record.put("score", m.second.first);
      char szWhere[64] = {0};
      snprintf(szWhere, sizeof(szWhere), "charid=%llu", m.first);
      pAct->m_strWhere = szWhere;
      pAct->m_eType = xSQLType_Update;
    }
    else
    {
      // 加分时, 更新得分时间
      record.put("charid", m.first);
      record.put("score", m.second.first);
      record.put("time", xTime::getCurUSec());
      pAct->m_eType = xSQLType_Replace;
    }
    thisServer->m_oSQLThread.add(pAct);

    XLOG << "[组队排位赛-积分保存], 保存成功, 玩家:" << m.first << "积分:" << m.second.first << XEND;
  }

  m_mapTempDirtyScore.clear();
}

void TeamPwsRank::timer(DWORD cur)
{
  if (!m_bUpdateRank || cur < m_dwUpdateTimeTick)
    return;

  m_bUpdateRank = false;
  m_dwUpdateTimeTick = cur + randBetween(300, 600); // 5分钟->10分钟更新一次

  saveRankData();
  XLOG << "[组队排位赛-排名保存], 定时保存成功, 下次保存时间:" << m_dwUpdateTimeTick << XEND;
}

void TeamPwsRank::saveRankData()
{
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, TEAM_PWS_TABLE_NAME);
  if (pField == nullptr)
    return;
  xTime frameTimer;
  // 更新排名
  DWORD rank = 1;
  for (auto &l : m_listScores)
  {
    xSQLAction* pAct = xSQLThread::create(pField);
    if (!pAct)
    {
      XERR << "[组队排位赛-更新排名], 创建xSQLAction失败, 玩家:" << l.qwCharID << "排名:" << rank++ << XEND;
      continue;
    }
    xRecord& record = pAct->m_oRecord;
    record.put("rank", rank++);

    char szWhere[64] = {0};
    snprintf(szWhere, sizeof(szWhere), "charid=%llu", l.qwCharID);
    pAct->m_strWhere = szWhere;
    pAct->m_eType = xSQLType_Update;
    thisServer->m_oSQLThread.add(pAct);
  }

  QWORD e = frameTimer.uElapse();
  XLOG << "[组队排位赛-排名保存], 耗时:" << e << XEND;
}

DWORD TeamPwsRank::getRankByCharID(QWORD charid)
{
  auto it = m_mapCharid2Rank.find(charid);
  return it != m_mapCharid2Rank.end() ? it->second : 0;
}

DWORD TeamPwsRank::getUserScore(QWORD charid) const
{
  auto it = m_mapCharID2Score.find(charid);
  if (it != m_mapCharID2Score.end())
    return it->second;
  return 0;
}

bool TeamPwsRank::checkAdd(DWORD score)
{
  if (m_listScores.empty())
    return true;
  return m_listScores.size() < m_dwMaxCacheNum || m_listScores.back().dwScore < score;
}

void TeamPwsRank::recordOutRankUser(QWORD charid, DWORD score)
{
  auto m = m_mapScore2UserSet.find(score);
  if (m != m_mapScore2UserSet.end())
  {
    m->second.insert(charid);
  }
  else
  {
    m_mapScore2UserSet[score].insert(charid);
  }
}

void TeamPwsRank::removeOutRankUser(QWORD charid, DWORD score)
{
  if (score == 0)
  {
    auto it = m_mapCharID2Score.find(charid);
    if (it == m_mapCharID2Score.end())
      return;
    score = it->second;
  }
  auto m = m_mapScore2UserSet.find(score);
  if (m != m_mapScore2UserSet.end())
  {
    m->second.erase(charid);
  }
}

void TeamPwsRank::recordUserScore(QWORD charid, DWORD score)
{
  auto it = m_mapCharID2Score.find(charid);
  if (it != m_mapCharID2Score.end())
  {
    it->second = score;
  }
  else
  {
    m_mapCharID2Score[charid] = score;
  }
  // 不记录m_listScores中的玩家
  if (!inRankList(charid))
  {
    auto m = m_mapScore2UserSet.find(score);
    if (m != m_mapScore2UserSet.end())
    {
      m->second.insert(charid);
    }
    else
    {
      m_mapScore2UserSet[score].insert(charid);
    }
  }
}

void TeamPwsRank::updateUserInRankList(SPwsScoreData& data, bool hasShowData/*=true*/)
{
  // assume m_dwValidSize=200, m_dwMaxCacheNum=1000
  auto it = find_if(m_listScores.begin(), m_listScores.end(), [&data](const SPwsScoreData& r) -> bool{
      return data.qwCharID == r.qwCharID;
      });
  if (it == m_listScores.end())
    return;

  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG();

  if (!hasShowData)
  {
    DWORD newscore = data.dwScore;
    data = *it;
    data.dwScore = newscore;
    data.eRank = rCFG.getERankByScore(newscore);
  }

  bool isFull = m_listScores.size() == m_dwMaxCacheNum;
  m_listScores.erase(it);

  if (m_listScores.empty())
  {
    m_listScores.push_back(data);
  }
  else if (data.dwScore == m_listScores.back().dwScore)
  {
    // 积分减少, 等于当前列表最后一名
    m_listScores.push_back(data);
  }
  else if (data.dwScore < m_listScores.back().dwScore)
  {
    // 积分减少, 小于当前列表最后一名
    if (isFull)
    {
      // 跌出1000名外, 需要寻找新的玩家作为第1000名
      for (auto m = m_mapScore2UserSet.rbegin(); m != m_mapScore2UserSet.rend(); ++m)
      {
        if (m->second.empty())
          continue;
        auto p = randomStlContainer(m->second);
        if (!p)
          continue;
        if (m->first <= data.dwScore)
        {
          // 依然是第1000名
          m_listScores.push_back(data);
        }
        else
        {
          // 新玩家作为第1000名
          QWORD newuserid = *p;
          DWORD newendscore = m->first;
          m->second.erase(newuserid);

          // 头像等数据, 不设置, 有玩家请求查看时, 从redis获取
          SPwsScoreData newdata(newuserid, newendscore);
          newdata.eRank = rCFG.getERankByScore(newendscore);
          m_listScores.push_back(newdata);

          // 更新列表玩家索引
          m_setAllRankUsers.erase(data.qwCharID);
          m_setAllRankUsers.insert(newuserid);
        }
        break;
      }
    }
    else
    {
      m_listScores.push_back(data);
    }
  }
  else if (data.dwScore > m_listScores.front().dwScore)
  {
    // 成为新的第一名
    m_listScores.push_front(data);
  }
  else
  {
    // 积分变化后, 积分仍在当前1000名内
    auto it = m_listScores.begin();
    for (auto s = m_listScores.begin(); s != m_listScores.end(); ++s)
    {
      if (s->dwScore < data.dwScore)
      {
        it = s;
        break;
      }
    }

    m_listScores.insert(it, data);
  }

  // 更新玩家->积分关系
  recordUserScore(data.qwCharID, data.dwScore);
}

void TeamPwsRank::insertNewData(const SPwsScoreData& data)
{
  // 调用之前需检查, data积分满足插入条件

  if (m_listScores.size() >= m_dwMaxCacheNum && !m_listScores.empty())
  {
    // 第1000名被挤出列表
    auto& r = m_listScores.back();

    m_setAllRankUsers.erase(r.qwCharID);
    recordOutRankUser(r.qwCharID, r.dwScore);

    m_listScores.pop_back();
  }

  if (m_listScores.empty())
  {
    m_listScores.push_back(data);
  }
  else if (m_listScores.front().dwScore < data.dwScore)
  {
    m_listScores.push_front(data);
  }
  else if (m_listScores.back().dwScore >= data.dwScore)
  {
    m_listScores.push_back(data);
  }
  else
  {
    auto it = m_listScores.begin();
    for (auto s = m_listScores.begin(); s != m_listScores.end(); ++s)
    {
      if (s->dwScore < data.dwScore)
      {
        it = s;
        break;
      }
    }

    m_listScores.insert(it, data);
  }

  m_setAllRankUsers.insert(data.qwCharID);
  // 新进入1000名的玩家, 移除在m_mapScore2UserSet中的记录
  removeOutRankUser(data.qwCharID);
  // 记录玩家最新得分
  recordUserScore(data.qwCharID, data.dwScore);
}

void TeamPwsRank::updateRankInfo(bool force)
{
  if (!force && !m_bTempRankChange)
    return;
  m_bTempRankChange = false;

  DWORD rank = 1;
  m_mapCharid2Rank.clear();
  for (auto &l : m_listScores)
  {
    if (rank > m_dwValidSize)
      break;
    m_mapCharid2Rank[l.qwCharID] = rank++;
  }
}

void TeamPwsRank::clear()
{
  m_listScores.clear();
  m_setAllRankUsers.clear();
  m_mapCharid2Rank.clear();
  m_mapScore2UserSet.clear();
}

void TeamPwsRank::execReward()
{
  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG();
  const vector<SPwsSeasonReward>& vecReward = rCFG.vecSeasonReward;
  if (vecReward.empty())
    return;

  DWORD rank = 1;
  for (auto &v : vecReward)
  {
    while(rank <= v.dwNeedRank)
    {
      if (m_listScores.empty())
        break;
      QWORD charid = m_listScores.front().qwCharID;
      // send mail to user
      MailManager::getMe().sendMail(charid, v.dwMailID);
      XLOG << "[组队排位赛-赛季奖励], 发送邮件成功, 玩家:" << charid << "邮件:" << v.dwMailID << "奖励区间:" << v.dwNeedRank << "排名:" << rank << XEND;

      m_listScores.pop_front();
      rank ++;
    }
  }

  // 清空内存积分数据
  clear();

  // 更新数据库
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, TEAM_PWS_TABLE_NAME);
  if (pField == nullptr)
    return;
  xSQLAction* pAct = xSQLThread::create(pField);
  if (!pAct)
  {
    XERR << "[ScoreManger-清除积分], 创建xSQLAction失败" << XEND;
    return;
  }
  xRecord& record = pAct->m_oRecord;
  record.put("score", 0);
  record.put("rank", 0);
  pAct->m_eType = xSQLType_Update;
  thisServer->m_oSQLThread.add(pAct);

  XLOG << "[组队排位赛-赛季积分清空], 清空成功" << XEND;
}

/**********************************************************************************/
/***************************ScoreManager*******************************************/
/**********************************************************************************/

ScoreManager::ScoreManager()
{

}

ScoreManager::~ScoreManager()
{

}

void ScoreManager::init()
{
  DWORD validnum = MiscConfig::getMe().getTeamPwsCFG().dwMaxRecordCnt;
  m_oTeamPwsRankData.setSize(validnum, validnum + 800);

  loadData();
  loadPwsSeasonData();
}

void ScoreManager::loadData()
{
  m_oTeamPwsRankData.loadData();
}

void ScoreManager::updatePwsUserScore(QWORD charid, int change)
{
  m_oTeamPwsRankData.changeScore(charid, change);
}

void ScoreManager::onUserOnline(DWORD zoneid, QWORD charid)
{
  DWORD score = getPwsScoreByCharID(charid);
  if (score)
  {
    SyncUserScoreMatchSCmd cmd;
    cmd.set_etype(EPVPTYPE_TEAMPWS);
    cmd.set_charid(charid);
    cmd.set_score(score);
    cmd.set_season(m_dwTeamPwsSeason);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToZone(zoneid, send, len);
  }
}

void ScoreManager::updateScore(const UpdateScoreMatchSCmd& cmd)
{
  m_oTeamPwsRankData.updateScore(cmd);
}

void ScoreManager::queryPwsRankInfo(DWORD zoneid, QWORD charid)
{
  QueryTeamPwsRankMatchCCmd cmd;
  DWORD rank = 1;
  DWORD num = m_oTeamPwsRankData.getValidSize();
  for (auto &l : m_oTeamPwsRankData.m_listScores)
  {
    if (l.haveShowData() == false)
      l.getDataByGChar();

    TeamPwsRankInfo* pInfo = cmd.add_rankinfo();
    if (pInfo == nullptr)
      continue;
    pInfo->set_name(l.strUserName);
    pInfo->mutable_portrait()->CopyFrom(l.oPortrait);
    pInfo->set_rank(rank++);
    pInfo->set_score(l.dwScore);
    pInfo->set_erank(l.eRank);
    pInfo->set_profession(l.eProfession);
    if (rank > num)
      break;
  }
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToClient(charid, zoneid, send, len);
}

// 组队数据
void ScoreManager::queryPwsTeamInfo(DWORD zoneid, QWORD charid, QueryTeamPwsTeamInfoMatchCCmd& cmd)
{
  DWORD rank = m_oTeamPwsRankData.getRankByCharID(charid);
  if (rank)
    cmd.set_myrank(rank);
  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG();
  for (int i = 0; i < cmd.userinfos_size(); ++i)
  {
    auto p = cmd.mutable_userinfos(i);
    if (!p)
      continue;
    DWORD score = getPwsScoreByCharID(p->charid());
    if (score == 0)
      continue;

    p->set_score(score);
    p->set_erank(rCFG.getERankByScore(score));
  }
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToClient(charid, zoneid, send, len);
}

DWORD ScoreManager::getPwsScoreByCharID(QWORD charid) const
{
  return m_oTeamPwsRankData.getUserScore(charid);
}

void ScoreManager::timer(DWORD cur)
{
  if (m_dwTeamPwsRewardTime && cur >= m_dwTeamPwsRewardTime)
  {
    // 赛季结束, 奖励时间
    m_dwTeamPwsRewardTime = 0;
    m_dwTeamPwsCount = 0;
    m_dwTeamPwsSeason ++;

    m_oTeamPwsRankData.execReward();
    savePwsSeasonData();
  }

  m_oTeamPwsRankData.timer(cur);
}


void ScoreManager::addPwsCount()
{
  m_dwTeamPwsCount ++;
  const STeamPwsCFG& rCFG = MiscConfig::getMe().getTeamPwsCFG();
  if (rCFG.dwSeasonBattleTimes == m_dwTeamPwsCount)
  {
    // 本赛季最后一次比赛
    m_dwTeamPwsRewardTime = now() + rCFG.dwRewardTime;
    XLOG << "[组队排位赛-设置奖励时间], 最后一次比赛, 设置奖励时间为:" << m_dwTeamPwsRewardTime << XEND;
  }
  XLOG << "[组队排位赛-比赛次数增加], 当前次数:" << m_dwTeamPwsCount << XEND;
  savePwsSeasonData();
}

void ScoreManager::loadPwsSeasonData()
{
  xField *pField = thisServer->getDBConnPool().getField(REGION_DB, "global");
  if (nullptr == pField)
  {
    XERR << "[组队排位赛-加载] 获取数据库失败" << XEND;
    return;
  }

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, "name = \'team_pws\'");
  if (ret == QWORD_MAX)
  {
    XERR << "[组队排位赛-加载] 查询失败, ret :" << ret << XEND;
    return;
  }

  Json::Value value;
  if (set.empty() == false)
  {
    const xRecord& record = set[0];
    string data = record.getString("data");
    Json::Reader reader;
    reader.parse(data, value);
  }
  m_dwTeamPwsSeason = value["season"].asUInt();
  m_dwTeamPwsCount = value["count"].asUInt();
  m_dwTeamPwsRewardTime = value["reward_time"].asUInt();

  // 意外停服重启后, 启动后延时一段时间执行
  if (m_dwTeamPwsRewardTime && m_dwTeamPwsRewardTime < now() + 60)
    m_dwTeamPwsRewardTime = now() + 60;
  // 设置赛季从1开始
  if (m_dwTeamPwsSeason == 0)
    m_dwTeamPwsSeason = 1;

  XLOG << "[组队排位赛-加载], 加载成功, 当前赛季已开展次数:" << m_dwTeamPwsCount << "奖励时间:" << m_dwTeamPwsRewardTime << XEND;
}

void ScoreManager::savePwsSeasonData()
{
  xField *pField = thisServer->getDBConnPool().getField(REGION_DB, "global");
  if (nullptr == pField)
  {
    XERR << "[组队排位赛-加载] 获取数据库失败" << XEND;
    return;
  }

  Json::Value value;
  value["season"] = m_dwTeamPwsSeason;
  value["count"] = m_dwTeamPwsCount;
  value["reward_time"] = m_dwTeamPwsRewardTime;
  Json::FastWriter writer;

  xRecord record(pField);
  record.put("name", "team_pws");
  record.put("data", writer.write(value));
  thisServer->getDBConnPool().exeReplace(record);

  XLOG << "[组队排位赛-更新数据], 更新成功, 次数:" << m_dwTeamPwsCount << "时间:" << m_dwTeamPwsRewardTime << XEND;
}

