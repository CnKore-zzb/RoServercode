#pragma once

#include "xDefine.h"
#include <set>

class SceneUser;

struct StageStepConfig
{
  DWORD stageID;
  DWORD stepID;
  DWORD type;     // 0普通 1精英
  std::string name;
  DWORD raidID;
  DWORD baseExp;
  DWORD jobExp;
  DWORD silver;
  DWORD gold;

  StageStepConfig()
  {
    stageID = 0;
    stepID = 0;
    type = 0;
    raidID = 0;
    baseExp = jobExp = silver = gold = 0;
  }
};

struct StageConfig
{
  DWORD stageID;
  std::string name;
  std::map<DWORD, DWORD> normalStarRewardList;
  std::map<DWORD, StageStepConfig> normalStepList;
  std::map<DWORD, StageStepConfig> hardStepList;

  StageConfig()
  {
    stageID = 0;
  }
};

struct StageStepID
{
  StageStepID()
  {
    stageID = stepID = 0;
    type = 0;
  }
  DWORD stageID;
  DWORD stepID;
  DWORD type;
};

enum StageType
{
  STAGE_TYPE_NORMAL = 0,
  STAGE_TYPE_HARD   = 1,
  STAGE_TYPE_MAX
};

class Stage
{
  public:
    Stage():m_dwStageID(0) {}
    virtual ~Stage() {}

    virtual StageType getType() = 0;
    virtual void getLastStep(DWORD &stageID, DWORD &step) = 0;

  public:
    DWORD m_dwStageID;                // 世界地图的关卡id
};

class StageNormal : public Stage
{
  public:
    StageNormal()
    {
    }
    virtual ~StageNormal()
    {
    }

    virtual StageType getType()
    {
      return STAGE_TYPE_NORMAL;
    }
    virtual void getLastStep(DWORD &stageID, DWORD &step);

  public:
    DWORD getStarNum()
    {
      DWORD ret = 0;
      for (auto it=m_list.begin(); it!=m_list.end(); ++it)
      {
        ret += it->second;
      }
      return ret;
    }

  public:
    std::map<DWORD, DWORD> m_list;    // 子关卡 星数
    std::set<DWORD> m_reward;         // 已领取的星数奖励
};

class StageHard : public Stage
{
  public:
    StageHard()
    {
    }
    virtual ~StageHard()
    {
    }

    virtual StageType getType()
    {
      return STAGE_TYPE_HARD;
    }
    virtual void getLastStep(DWORD &stageID, DWORD &step);

    struct StageHardItem
    {
      StageHardItem()
      {
        m_dwFinish = m_dwRestTime = 0;
      }
      DWORD m_dwFinish;    // 是否通关
      DWORD m_dwRestTime;  // 剩余挑战次数
    };

  public:
    std::map<DWORD, StageHardItem> m_list;    // 子关卡id
};

namespace Cmd
{
  //class BlobUserData;
  class BlobStage;
};

class UserStage
{
  public:
    UserStage(SceneUser *u);
    ~UserStage();

  public:
    void load(const Cmd::BlobStage &data);
    void save(Cmd::BlobStage *data);

    Stage* getStage(DWORD stageID, StageType type);
    Stage* addStage(DWORD stageID, StageType type);

    void getNextStep(DWORD &stageID, DWORD &stepID, StageType type);
    void send();
    void send(DWORD stageID);
    void start(DWORD stageID, DWORD stepID, DWORD type);

    // normal
    void getReward(DWORD stageID, DWORD starID);
    void addStar(DWORD dmapID, DWORD starID);

  private:
    SceneUser *m_pUser;

  private:
    std::map<DWORD, Stage*> m_list[STAGE_TYPE_MAX];    // StageID Stage
    DWORD m_dwResetTime;

  public:
    static bool loadConfig();
    static StageConfig* getStageConfig(DWORD stageID);
    static StageStepConfig* getStageStepConfig(DWORD stageID, DWORD stepID, StageType type);
  public:
    static std::map<DWORD, StageConfig> s_cfg;
    static std::map<DWORD, StageStepID> s_raidStage;
};
