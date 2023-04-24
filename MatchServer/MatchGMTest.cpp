#include "MatchGMTest.h"
#include "MatchServer.h"
#include "ScoreManager.h"

MatchGMTest::MatchGMTest()
{

}

MatchGMTest::~MatchGMTest()
{

}

void MatchGMTest::gmTest(const SceneGMTestMatchSCmd& cmd)
{
  switch(cmd.etype())
  {
    case EMATCHGM_JOINTEAMPWS:
      {
        MatchManager::getMe().openTeamPws();

        SSceneGMTestMatchData& data = m_mapTestCmd[cmd.etype()];
        data.dwEndTestTime = cmd.lasttime() + now();
        data.oData.CopyFrom(cmd);
        XDBG << "[Match-GMTest], 组队排位赛报名测试开始" << cmd.ShortDebugString() << XEND;
      }
      break;
    default:
      break;
  }
}

void MatchGMTest::timer(DWORD cur)
{
  auto exec = [&](SceneGMTestMatchSCmd& data)
  {
    switch(data.etype())
    {
      case EMATCHGM_JOINTEAMPWS:
        {
          JoinTeamPwsMatchSCmd testcmd;
          testcmd.set_teamid(randBetween(1000,1000000));
          testcmd.set_zoneid(randBetween(10001,10060)*1000+1);
          QWORD charid = 4313682203 + randBetween(1, 1000000);
          testcmd.set_leaderid(charid);
          DWORD size = randBetween(1,100) > 30 ? randBetween(1,5) : 6;
          for (DWORD i = 0; i < size; ++i)
            testcmd.add_members(charid+i);

          DWORD minscore = data.params_size() > 0 ? data.params(0) : 0;
          DWORD maxscore = data.params_size() > 1 ? data.params(1) : 1000;
          testcmd.set_avescore(randBetween(minscore, maxscore));

          MatchManager::getMe().joinTeamPws(testcmd);
        }
        break;
      default:
        break;
    }
  };

  for (auto m = m_mapTestCmd.begin(); m != m_mapTestCmd.end(); )
  {
    if (cur >= m->second.dwEndTestTime)
    {
      m = m_mapTestCmd.erase(m);
      continue;
    }
    if (cur >= m->second.dwNextTestTime)
    {
      // 1秒多次
      if (m->second.oData.frequency())
      {
        for (DWORD i = 0; i < m->second.oData.frequency(); ++i)
          exec(m->second.oData);
      }
      else
      {
        exec(m->second.oData);
      }
      m->second.dwNextTestTime = cur + m->second.oData.interval();
    }
    ++m;
  }

  /*
  static DWORD delay = 10;
  if (delay > 1)
  {
    delay --;
    return;
  }

  static TSetQWORD charids;
  if (charids.empty())
  {
    xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
    if (pField)
    {
      pField->setValid("charid");
      xRecordSet set;
      QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, nullptr, nullptr);
      for (QWORD i = 0; i < ret; ++i)
      {
        charids.insert(set[i].get<QWORD>("charid"));
      }
      XLOG << "[gsj init], charidsize:" << charids.size() << XEND;
    }
  }

  static map<QWORD, DWORD> usercnt;
  static DWORD test = 0;
  static DWORD testsize = 8000;
  while(test++ < testsize)
  {
    UpdateScoreMatchSCmd cmd;
    TSetQWORD tmpset;
    for (DWORD i = 0; i < 12; ++i)
    {
      QWORD id= 0;
      while(true)
      {
        auto p = randomStlContainer(charids);
        if (!p)
          continue;
        id = *p;
        if (usercnt[id] >= 60)
          continue;
        if (tmpset.find(id) != tmpset.end())
          continue;

        break;
      }

      usercnt[id] ++;
      if (usercnt[id] == 60)
        charids.erase(id);

      tmpset.insert(id);
      MatchScoreData* pdata = cmd.add_userscores();
      pdata->set_charid(id);
      if (i < 6)
        pdata->set_score(randBetween(1,111));
      else
      {
        DWORD oldscore = ScoreManager::getMe().getPwsScoreByCharID(id);
        if (oldscore > 1200)
        {
          if (randBetween(1,100) > 50)
            pdata->set_score(0 - (int)randBetween(1,50));
          else
            pdata->set_score(randBetween(1,111));
        }
      }
    }

    ScoreManager::getMe().updateScore(cmd);
  }
  */
}
