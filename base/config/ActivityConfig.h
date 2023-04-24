#pragma once

#include "xSingleton.h"
#include "xLuaTable.h"
#include <vector>
#include "ActivityCmd.pb.h"
using std::map;
using std::string;
using std::vector;

enum EActLimitType
{
  EACTLIMITTYPE_MIN = 0,
  EACTLIMITTYPE_SYS = 1,
  EACTLIMITTYPE_PLAYER = 2,
  EACTLIMITTYPE_RANDOMMAP = 3,  //随机地图发送
  EACTLIMITTYPE_NO = 4,     //无
  EACTLIMITTYPE_MAX = 5
};

enum EActivityType
{
  EACTIVITYTYPE_BCAT = 1,             //b格猫
  EACTIVITYTYPE_CRAZYGHOST = 2,			 //幽灵暴走事件
  EACTIVITYTYPE_QUEST = 3,            //问答npc
  EACTIVITYTYPE_NORMAL = 4,           //普通活动
  EACTIVITYTYPE_MONEYCAT = 5,         //招财猫活动
  EACTIVITYTYPE_CAPRA_OUT = 6,         //卡普拉场外活动
  EACTIVITYTYPE_CAPRA_IN = 7,         //卡普拉场内活动
  EACTIVITYTYPE_POLLY = 8,            //波利乱斗
  EACTIVITYTYPE_MVPBATTLE = 9,        //mvp竞争战
  EACTIVITYTYPE_TEAMPWS = 10,         //组队排位赛
  EACTIVITYTYPE_MAX = 11
};


enum EACTNODE_TYPE
{
  EACTNODETYPE_STAGE = 1,
  EACTNODETYPE_SEQ = 2,
  EACTNODETYPE_PAR = 3,
};

struct SActvityNodeBase
{
  EACTNODE_TYPE type;
  string content;
  xLuaData oParams;
  SActvityNodeBase() {}
};

struct ActivityStageItem
{
  DWORD id = 0;
  SActvityNodeBase stageNode;
  std::vector<SActvityNodeBase> vecNode;     
  ActivityStageItem() {}
};
typedef vector<ActivityStageItem> TVecSActivityStageItem;

typedef vector<struct tm> TVecSStartTime;

struct SActivityCFG
{
public:
  bool checkMap(DWORD mapId) const { auto it = std::find(m_vecMapId.begin(), m_vecMapId.end(), mapId); return it != m_vecMapId.end(); }
  DWORD randomMap() const
  {
    DWORD* pId = randomStlContainer(m_vecMapId);
    if (pId == nullptr)
      return 0;
    return *pId;
  }
  DWORD m_dwId = 0;   //activity id
  std::string m_strName;
  EActLimitType m_limitType;
  EActivityType m_actType;
  std::vector<DWORD> m_vecMapId;
  xLuaData m_condition;
  TVecSStartTime m_vecStartTime;
  DWORD m_duration = 0;  
  DWORD m_dwPath = 0;
  TVecDWORD m_vecUnshowMap;
  TVecSActivityStageItem m_vecStage;
  SActivityCFG() {}
};

enum EGlobalActLimitType
{
  EGlobalActLimitType_Min = 0,
  EGlobalActLimitType_Acc = 1,
  EGlobalActLimitType_Char = 2,
  EGlobalActLimitType_Max = 3,
};

struct SGlobalActCFG
{
  DWORD m_dwId = 0;
  GlobalActivityType m_actType;
  string m_strName;
  DWORD m_dwLimitCount = 0;
  EGlobalActLimitType m_eLimitType = EGlobalActLimitType_Min;

  DWORD getDepositId() const;
  DWORD getParam(DWORD dwIndex) const { if (dwIndex >= m_vecParam.size()) return 0; else return m_vecParam[dwIndex]; }
  bool isParamExist(DWORD value) const;
  TVecDWORD m_vecParam;
  float getDiscount(DWORD serviceid) const;
};

class ActivityConfig : public xSingleton<ActivityConfig>
{
  public:
    ActivityConfig();
    virtual ~ActivityConfig();

    bool loadConfig();
    bool loadStepConfig();
    bool loadGlobalActConfig();

    const SActivityCFG* getActivityCFG(DWORD dwID) const;
    const SGlobalActCFG* getGlobalActCFG(DWORD dwID) const;
    template<class T>
    void foreach(T func) { for_each(m_mapActivityCFG.begin(), m_mapActivityCFG.end(), [func](map<DWORD, SActivityCFG>::value_type& r) {func(r.first, r.second);}); }

    void markActivityOpen(const string& name) { m_setActivityNames.insert(name); }
    void markActivityClose(const string& name) { m_setActivityNames.erase(name); }
    bool hasActivity(const string& name) const { return m_setActivityNames.find(name) != m_setActivityNames.end(); }

    bool openNpcFunc(DWORD npcid, const TSetDWORD& funcSet);
    bool closeNpcFunc(DWORD npcid, const TSetDWORD& funcSet);
    bool checkNpcFuncClose(DWORD npcid, DWORD funcid) const;
  private:
    map<DWORD, SActivityCFG> m_mapActivityCFG;
    std::set<string> m_setActivityNames;
    map<DWORD, TSetDWORD> m_mapCloseNpcFunctions;
    map<DWORD, SGlobalActCFG> m_mapGlobalActCFG;
};

