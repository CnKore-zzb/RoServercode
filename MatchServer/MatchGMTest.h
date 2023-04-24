#pragma once
#include "MatchManager.h"

struct SSceneGMTestMatchData
{
  DWORD dwNextTestTime = 0; // 下次执行时间
  DWORD dwEndTestTime = 0; // 结束测试时间
  SceneGMTestMatchSCmd oData;
};

class MatchGMTest : public xSingleton<MatchGMTest>
{
  public:
    MatchGMTest();
    virtual ~MatchGMTest();
  public:
    void gmTest(const SceneGMTestMatchSCmd& cmd);
    void timer(DWORD cur);

  private:
    std::map<ESceneGMMatchType,SSceneGMTestMatchData> m_mapTestCmd;
};
