#include "GuildGvg.h"
#include "Guild.h"
#include "MatchSCmd.pb.h"
#include "GuildServer.h"
#include "MiscConfig.h"

GuildGvg::GuildGvg(Guild* pGuild) : m_pGuild(pGuild)
{

}

GuildGvg::~GuildGvg()
{

}

bool GuildGvg::fromData(const BlobGGvg& data)
{
  if (data.partin_time())
  {
    m_oPartinTime2Users.first = data.partin_time();
    for (int i = 0; i < data.partin_users_size(); ++i)
    {
      m_oPartinTime2Users.second.insert(data.partin_users(i));
    }
  }

  m_dwSuperGvgTime = data.sugvgtime();
  m_dwSuperGvgCnt = data.sugvgcnt();
  m_dwSuperGvgScore = data.sugvgscore();
  m_dwVersion = data.version();

  if (m_dwSuperGvgTime && m_dwSuperGvgTime > now())
    m_dwSuperBeginTime = m_dwSuperGvgTime - 35 * 60;
  return true;
}

bool GuildGvg::toData(BlobGGvg* pData)
{
  if (pData == nullptr)
    return false;
  if (!m_oPartinTime2Users.second.empty())
  {
    pData->set_partin_time(m_oPartinTime2Users.first);
    for (auto &s : m_oPartinTime2Users.second)
    {
      pData->add_partin_users(s);
    }
  }

  if (m_dwSuperGvgTime && m_dwSuperGvgTime > now())
    pData->set_sugvgtime(m_dwSuperGvgTime);

  pData->set_sugvgcnt(m_dwSuperGvgCnt);
  pData->set_sugvgscore(m_dwSuperGvgScore);

  //新建公会不执行version.()
  pData->set_version(GVG_DATA_VERSION);

  return true;
}

void GuildGvg::addGvgPartinUser(QWORD userid)
{
  DWORD partintime = now() / DAY_T;

  if (m_oPartinTime2Users.first != partintime)
  {
    m_oPartinTime2Users.first = partintime;
    m_oPartinTime2Users.second.clear();
  }

  m_oPartinTime2Users.second.insert(userid);
  //m_pGuild->setMark(EGUILDDATA_MISC);
  XLOG << "[公会-gvg], 玩家达成gvg参战条件, 公会:" << m_pGuild->getGUID() << m_pGuild->getName() << "玩家:" << userid << XEND;
}

const TSetQWORD& GuildGvg::getGvgPartInUsers()
{
  if (m_oPartinTime2Users.first != now() / DAY_T)
    m_oPartinTime2Users.second.clear();

  return m_oPartinTime2Users.second;
}

void GuildGvg::joinSuperGvg(DWORD begintime)
{
  m_dwSuperBeginTime = begintime;
  m_dwSuperGvgTime = begintime + 35 * 60;

  m_pGuild->setMark(EGUILDDATA_SUPERGVG);
  m_pGuild->updateData(true);

  JoinSuperGvgMatchSCmd cmd;
  cmd.set_guildid(m_pGuild->getGUID());
  cmd.set_zoneid(m_pGuild->getZoneID());
  cmd.set_guildname(m_pGuild->getName());
  cmd.set_guildicon(m_pGuild->getPortrait());
  cmd.set_firecount(m_dwSuperGvgCnt);
  cmd.set_firescore(m_dwSuperGvgScore);
  cmd.set_begintime(m_dwSuperBeginTime);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToZone(m_pGuild->getZoneID(), send, len);

  m_pGuild->broadcastMsg(25502);
  m_pGuild->broadcastMsg(25503);
  XLOG << "[公会-gvg], 参加公会战决战, 公会:" << m_pGuild->getGUID() << m_pGuild->getName() << "结束时间:" << m_dwSuperGvgTime << XEND;
}

void GuildGvg::endSuperGvg()
{
  m_dwSuperGvgTime = 0;

  m_pGuild->setMark(EGUILDDATA_SUPERGVG);
  m_pGuild->updateData();
  XLOG << "[公会-gvg], 结束公会战决战, 公会:" << m_pGuild->getGUID() << m_pGuild->getName() << XEND;
}

void GuildGvg::finishSuperGvg(DWORD rank)
{
  // 初始积分
  if (m_dwSuperGvgCnt == 0)
  {
    m_dwSuperGvgCnt = 1;
    m_dwSuperGvgScore = 1;
  }

  // 更新前段位
  DWORD orilv = getSuperGvgLv();

  m_dwSuperGvgCnt ++;
  DWORD score = MiscConfig::getMe().getSuperGvgCFG().getScoreByRank(rank);
  m_dwSuperGvgScore += score;
  m_pGuild->setMark(EGUILDDATA_MISC);

  // 更新段位
  if (getSuperGvgLv() != orilv)
  {
    m_pGuild->setMark(EGUILDDATA_SUPERGVG_LV);
    m_pGuild->updateData();
  }

  GuildEventM& rEvent = m_pGuild->getEvent();
  TVecString vecParams;

  DWORD dwNow = now();
  stringstream sstr;
  sstr << xTime::getYear(dwNow);
  vecParams.push_back(sstr.str());
  sstr.str("");
  sstr << xTime::getMonth(dwNow);
  vecParams.push_back(sstr.str());
  sstr.str("");
  sstr << xTime::getDay(dwNow);
  vecParams.push_back(sstr.str());
  sstr.str("");
  sstr << rank;
  vecParams.push_back(sstr.str());

  rEvent.addEvent(EGUILDEVENT_SUPERGVG_RANK, vecParams);

  XLOG << "[公会-gvg], 公会战完成, 统计排名, 公会:" << m_pGuild->getGUID() << m_pGuild->getName() << "排名:" << rank << "更新后排名数据:" << m_dwSuperGvgCnt << m_dwSuperGvgScore << XEND;
}

void GuildGvg::timer(DWORD curTime)
{
  if (!m_dwSuperGvgTime)
    return;

  if (curTime >= m_dwSuperGvgTime)
  {
    endSuperGvg();
    return;
  }

  // 25开战, 27, 29提示
  if (m_dwSuperBeginTime && curTime >= m_dwSuperBeginTime)
  {
    DWORD time = curTime - m_dwSuperBeginTime;
    if (time <= 4 * 60 && time % 120 == 0)
      m_pGuild->broadcastMsg(25517);
  }
}

/*强制设置胜率信息, 测试使用*/
void GuildGvg::setFireCntAndScore(DWORD cnt, DWORD score)
{
  XLOG << "[公会-gvg], 设置参战次数, 积分成功, 公会:" << m_pGuild->getGUID() << m_pGuild->getName() << "设置前数据:" << m_dwSuperGvgCnt << m_dwSuperGvgScore << "设置后数据:" << cnt << score << XEND;

  m_dwSuperGvgCnt = cnt;
  m_dwSuperGvgScore = score;
  m_pGuild->setMark(EGUILDDATA_MISC);
}

DWORD GuildGvg::getSuperGvgLv() const
{
  float avescore = 0;
  if (m_dwSuperGvgCnt)
    avescore = (float)m_dwSuperGvgScore / (float)m_dwSuperGvgCnt;
  return MiscConfig::getMe().getSuperGvgCFG().getLevelByScore(avescore);
}

void GuildGvg::version_1()
{
  if (m_dwSuperGvgCnt != 0)
  {
    ItemInfo item;
    item.set_id(5543);
    float avescore = (float)m_dwSuperGvgScore / (float)m_dwSuperGvgCnt;
    if (avescore <= 3)
      item.set_count(100);
    else if (avescore <= 5)
      item.set_count(200);
    else
      item.set_count(300);

    m_pGuild->getPack().addItem(item);
  }

  XLOG << "[公会-gvg], 清空决战数据, 公会:" << m_pGuild->getGUID() << "次数:" << m_dwSuperGvgCnt << "积分:" << m_dwSuperGvgScore << XEND;
  m_dwSuperGvgCnt = 0;
  m_dwSuperGvgScore = 0;
}

void GuildGvg::checkVersion()
{
  for (DWORD i = m_dwVersion; i < GVG_DATA_VERSION; ++i)
  {
    if (i == 0)
      version_1();
  }
  m_dwVersion = GVG_DATA_VERSION;
}
