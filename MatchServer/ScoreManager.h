#pragma once

#include "xSingleton.h"
#include <list>
#include <map>
#include "xDefine.h"
#include "xTime.h"
#include "MatchSCmd.pb.h"
#include "json/json.h"

const DWORD TEAMPWS_RANK_USERNUM = 200;
#define TEAM_PWS_TABLE_NAME "team_pws"

using namespace Cmd;
using namespace std;

using std::map;
using Json::Value;
using std::string;

struct SPwsScoreData
{
  // base data
  QWORD qwCharID = 0;
  DWORD dwScore = 0;
  ETeamPwsRank eRank = ETEAMPWSRANK_NONE;

  // user show data
  EProfession eProfession = EPROFESSION_MIN;
  UserPortraitData oPortrait;
  string strUserName;

  bool haveShowData() { return !strUserName.empty(); }
  void getDataByGChar();

  SPwsScoreData() {}
  SPwsScoreData(QWORD charid, DWORD score) : qwCharID(charid), dwScore(score) {}
};
typedef std::list<SPwsScoreData> TListScoreData;

class TeamPwsRank
{
  friend class ScoreManager;
  public:
    TeamPwsRank();
    ~TeamPwsRank();

  public:
    void setSize(DWORD vaildnum, DWORD cachenum) { m_dwValidSize = vaildnum; m_dwMaxCacheNum = cachenum; }
    void timer(DWORD cur);

    void loadData();
    void saveRankData();

    void updateScore(const UpdateScoreMatchSCmd& cmd);
    void changeScore(QWORD charid, int change);

    DWORD getRankByCharID(QWORD charid);
    DWORD getUserScore(QWORD charid) const;

    DWORD getValidSize() { return m_dwValidSize; }
  private:
    void clear();
    void execReward();
    void changeScore(QWORD charid, int change, const MatchScoreData& sdata);
    bool inRankList(QWORD charid) { return m_setAllRankUsers.find(charid) != m_setAllRankUsers.end();}

    // 保存积分数据
    void saveScoreData();

    // 是否可以插入
    bool checkAdd(DWORD score);

    // 玩家被挤出1000名时记录
    void recordOutRankUser(QWORD charid, DWORD score);
    // 玩家进入前1000名时移除
    void removeOutRankUser(QWORD charid, DWORD score = 0);

    // 记录玩家->得分关系
    void recordUserScore(QWORD charid, DWORD score);

    // 更新前200名玩家快速查询map : m_mapCharid2Rank
    void updateRankInfo(bool force = false);

    // 新得分的玩家, 超过第1000名, 需要插入列表
    void insertNewData(const SPwsScoreData& data);

    // 前1000名的玩家积分发送变化, hasShowData=true时, 使用data中数据更新玩家头像信息等
    void updateUserInRankList(SPwsScoreData& data, bool hasShowData = true);

  private:
    // 实际所需的排名数量
    DWORD m_dwValidSize = 0;

    // 最大记录的数量
    DWORD m_dwMaxCacheNum = 0;

    // 从大到小, 排名列表
    TListScoreData m_listScores;
    // m_listScores 中的所有玩家
    TSetQWORD m_setAllRankUsers;

    // 所有玩家id->rank, 便于查询, 只记录m_dwValidSize个
    map<QWORD, DWORD> m_mapCharid2Rank;

    // 所有玩家积分
    map<QWORD, DWORD> m_mapCharID2Score;
    // 积分->玩家列表(不包含m_listScores中的玩家)
    map<DWORD, TSetQWORD> m_mapScore2UserSet;

    // 积分变化时, 是否影响排名列表
    bool m_bTempRankChange = false;
    // 更新数据库标记
    bool m_bUpdateRank = false;
    // 更新数据库时间戳
    DWORD m_dwUpdateTimeTick = 0;

    // 待保存的积分数据 (charid, (score, dec/add 1/0))
    map<QWORD, pair<DWORD, bool>> m_mapTempDirtyScore;
};

class ScoreManager : public xSingleton<ScoreManager>
{
  public:
    ScoreManager();
    virtual ~ScoreManager();

  public:
    void timer(DWORD cur);
    void init();
    void updateScore(const UpdateScoreMatchSCmd& cmd);
    void onUserOnline(DWORD zoneid, QWORD charid);
    void queryPwsRankInfo(DWORD zoneid, QWORD charid);
    void queryPwsTeamInfo(DWORD zoneid, QWORD charid, QueryTeamPwsTeamInfoMatchCCmd& cmd);
    DWORD getPwsScoreByCharID(QWORD charid) const;
    void updatePwsUserScore(QWORD charid, int change);

    void addPwsCount();
  private:
    void loadData();

    void loadPwsSeasonData();
    void savePwsSeasonData();

  private:
    // 组队排位赛
    TeamPwsRank m_oTeamPwsRankData;

  private:
    DWORD m_dwTeamPwsSeason = 0; // 赛季
    DWORD m_dwTeamPwsCount = 0; // 当前赛季已开战次数
    DWORD m_dwTeamPwsRewardTime = 0; // 当前赛季结束, 设定的奖励时间
};

