#pragma once

#include <set>
#include <string>
#include "xLuaTable.h"
#include "SceneDefine.h"
#include "xSceneEntry.h"

using std::set;
using std::string;
using std::vector;

class SceneNpc;
class xSceneEntry;
// typedef std::unordered_set<xSceneEntry*> xSceneEntrySet;

// AIAction - base
class AIAction
{
  public:
    AIAction(const xLuaData& params) : m_oParams(params) { m_dwPercent = m_oParams.getTableInt("odds"); }
    virtual ~AIAction() {}

    virtual void doAction(SceneNpc *pNpc, xSceneEntrySet &targetSet) = 0;

    void setName(const string& name) { m_strName = name; }
    const string& getName() const { return m_strName; }
    DWORD getPerCent() const { return m_dwPercent; }
    DWORD getStep() const { return m_oParams.getTableInt("ai_step_index"); }
  protected:
    xLuaData m_oParams;
    string m_strName;
    DWORD m_dwPercent = 0;
};

// AIAction - talk
class TalkAIAction : public AIAction
{
  public:
    TalkAIAction(const xLuaData& params);
    virtual ~TalkAIAction() {}
    void doAction(SceneNpc *pNpc, xSceneEntrySet &targetSet);
  private:
    DWORD talkId = 0;
};

// AIAction - summon
class SummonNpcAIAction : public AIAction
{
  public:
    SummonNpcAIAction(const xLuaData& params);
    virtual ~SummonNpcAIAction() {}

    void doAction(SceneNpc *pNpc, xSceneEntrySet &targetSet);
  private:
    NpcDefine m_oNpcDef;
    DWORD m_dwNum = 0;
    bool m_bSupply = false;
};

// AIAction - gm
class GMCmdAIAction : public AIAction
{
  public:
    GMCmdAIAction(const xLuaData& params) : AIAction(params) {}
    virtual ~GMCmdAIAction() {}

    void doAction(SceneNpc *pNpc, xSceneEntrySet &targetSet);
};

// AIAction - skill
class SkillAIAction : public AIAction
{
  public:
    SkillAIAction(const xLuaData& params) : AIAction(params) { m_dwSkillID = m_oParams.getTableInt("skillID"); }
    virtual ~SkillAIAction() {}

    void doAction(SceneNpc *pNpc, xSceneEntrySet &targetSet);
  private:
    DWORD m_dwSkillID = 0;
};

// AIAction - skill me
class SkillMeAIAction : public AIAction
{
  public:
    SkillMeAIAction(const xLuaData& params) : AIAction(params) { m_dwSkillID = m_oParams.getTableInt("skillID"); }
    virtual ~SkillMeAIAction() {}

    void doAction(SceneNpc *pNpc, xSceneEntrySet &targetSet);
  private:
    DWORD m_dwSkillID = 0;
};

// AIAction - Route
class RouteAIAction : public AIAction
{
  public:
    RouteAIAction(const xLuaData& params);
    virtual ~RouteAIAction();

    void doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet);
  private:
    DWORD m_dwIndex = 0;
    bool m_bChange = true;
    vector<xPos> m_vecPos;
};

// AIAction - check servants -> relive
class SupplyAIAction : public AIAction
{
  public:
    SupplyAIAction(const xLuaData& params);
    virtual ~SupplyAIAction();

    void doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet);
};

// AIAction -  attack 攻击目标
class AttackAIAction : public AIAction
{
public:
  AttackAIAction(const xLuaData& params);
  virtual ~AttackAIAction();

  void doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet);
};

// AIAction -  getaway 逃离目标
class GetawayAIAction : public AIAction
{
public:
  GetawayAIAction(const xLuaData& params);
  virtual ~GetawayAIAction();

  void doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet);
private:
  float m_fDistance;
};

// AIAction -  buff 获得buff
class BuffAIAction : public AIAction
{
public:
  BuffAIAction(const xLuaData& params);
  virtual ~BuffAIAction();

  void doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet);

private:
  DWORD m_dwBuffId;
};

// AIAction -  trigger other conditions
class TriggerCondAIAction : public AIAction
{
public:
  TriggerCondAIAction(const xLuaData& params);
  virtual ~TriggerCondAIAction();

  void doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet);
private:
  std::string m_cond;
};

// AIAction -  expression 使用表情
class ExpressionAIAction : public AIAction
{
public:
  ExpressionAIAction(const xLuaData& params);
  virtual ~ExpressionAIAction();

  void doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet);
private:
  std::string  m_strExpression;
};

// AIAction -  changeai 改变AI
class ChangeAIAction : public AIAction
{
public:
  ChangeAIAction(const xLuaData& params);
  virtual ~ChangeAIAction();

  void doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet);
private:
  TSetDWORD  m_setAI;
};

// AIAction - set dest pos
class GoPosAIAction : public AIAction
{
  public:
    GoPosAIAction(const xLuaData& params);
    virtual ~GoPosAIAction();

    void doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet);
  private:
    xPos m_destPos;
};

// AIAction - 变成其他怪物
class TurnMonsterAIAction : public AIAction
{
  public:
    TurnMonsterAIAction(const xLuaData& params);
    virtual ~TurnMonsterAIAction();

    void doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet);

  private:
    NpcDefine m_oNpcDef;
};

// AIAction - 切换攻击目标
class ChangeTargetAIAction : public AIAction
{
  public:
    ChangeTargetAIAction(const xLuaData& params);
    virtual ~ChangeTargetAIAction();

    void doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet);
};

// AIAction - 复活尸体
class ReliveDeadAIAction : public AIAction
{
  public:
    ReliveDeadAIAction(const xLuaData& params);
    virtual ~ReliveDeadAIAction();

    void doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet);
};

// AIAction - 直接消失
class SetClearAIAction : public AIAction
{
  public:
    SetClearAIAction(const xLuaData& params);
    virtual ~SetClearAIAction();

    void doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet);
};

// AIAction - 走向我
class ComeToMeAIAction : public AIAction
{
  public:
    ComeToMeAIAction(const xLuaData& params);
    virtual ~ComeToMeAIAction();

    void doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet);
  private:
    float m_fDistance = 0;
};

// AIAction - reward by map user
struct SAIRewardData
{
  DWORD dwMinUserNum = 0;
  DWORD dwMaxUserNum = 0;
  TSetDWORD setRewards;
};
class RewardMapAIAction : public AIAction
{
  public:
    RewardMapAIAction(const xLuaData& params);
    virtual ~RewardMapAIAction();

    void doAction(SceneNpc* pNpc, xSceneEntrySet &targetSet);
  private:
    float m_fDropRange = 0;
    float m_fUserRange = 0;
    vector<SAIRewardData> m_vecRewards;
};

// AIAction - being_skill 生命技能
class BeingSkillAIAction : public AIAction
{
  public:
    BeingSkillAIAction(const xLuaData& params) : AIAction(params) { m_dwSkillGroupID = m_oParams.getTableInt("skillGroupID"); }
    virtual ~BeingSkillAIAction() {}

    void doAction(SceneNpc* pNpc, xSceneEntrySet& targetSet);
  private:
    DWORD m_dwSkillGroupID = 0;
};

// AIAction - 锁定目标
class LockTargetAIAction : public AIAction
{
  public:
    LockTargetAIAction(const xLuaData& params);
    virtual ~LockTargetAIAction() {};

    void doAction(SceneNpc* pNpc, xSceneEntrySet& targetSet);
};

// AIAction - 使目标放技能
class TargetSkillAIAction : public AIAction
{
  public:
    TargetSkillAIAction(const xLuaData& params) : AIAction(params) { m_dwSkillID = m_oParams.getTableInt("skillID"); }
    virtual ~TargetSkillAIAction() {}

    void doAction(SceneNpc *pNpc, xSceneEntrySet &targetSet);
  private:
    DWORD m_dwSkillID = 0;
};

class IceSpurtAIAction : public AIAction
{
  public:
    IceSpurtAIAction(const xLuaData& params);
    virtual ~IceSpurtAIAction();

    void doAction(SceneNpc* pNpc, xSceneEntrySet& targetSet);

  private:
    float m_fWidth = 0;
    DWORD m_dwMasterSkillID = 0;
    DWORD m_dwEnemySkillID = 0;
};

// AIAction - 向目标移动
class ChaseAIAction : public AIAction
{
  public:
    ChaseAIAction(const xLuaData& params);
    virtual ~ChaseAIAction();

    void doAction(SceneNpc* pNpc, xSceneEntrySet& targetSet);

  private:
    bool m_bCancel = 0;
};
