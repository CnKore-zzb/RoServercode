/**
 * @file ActivityManager.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version 
 * @date 2016-09-06
 */

#pragma once

#include "xSingleton.h"
#include "SessionCmd.pb.h"
#include "xLuaTable.h"
#include <vector>
#include <list>
#include <queue>
#include "SceneInterlocution.pb.h"
#include "ActivityCmd.pb.h"
#include "SceneDefine.h"
class Scene;
class SceneNpc;
struct SActivityCFG;
class ActivityStageBase;
class SceneUser;
enum EActivityState
{
  EActivityState_none = 1,
  EActivityState_start = 2,
  EActivityState_process = 3,
  EActivityState_stop = 4,
  EActivityState_expire = 5,
};

template <typename B>
struct TypeItem
{
public:
  std::string m_strName;
  TypeItem() {}
  virtual ~TypeItem() {}
  virtual B* create(const xLuaData &data) = 0;
};

template <typename B, typename T>
class TypeCreator : public TypeItem<B>
{
  using TypeItem<B>::m_strName;
public:
  TypeCreator(const char *s) :TypeItem<B>()
  {
    m_strName = s;
  }
  B* create(const xLuaData &data)
  {
    T *pT = NEW T(data);
    pT->setName(m_strName);    
    return pT;
  }
};

template <typename T>
class TypeItemFactory : public xSingleton<TypeItemFactory<T> >
{
  friend class xSingleton<TypeItemFactory<T> >;

public:
  ~TypeItemFactory()
  {
    final();
  }
  void final()
  {
    for (auto it = list.begin(); it != list.end(); ++it)
      SAFE_DELETE(it->second);
    list.clear();
  }
  void reg(TypeItem<T> *item)
  {
    if (!item) return;
    auto it = list.find(item->m_strName);
    if(list.end() != it)
    {
      printf("the same key for list, %s, %d \n", __FILE__, __LINE__);
      SAFE_DELETE(it->second);
    }

    list.insert(std::make_pair(item->m_strName, item));
  }

  T* create(const string& t, const xLuaData &data)
  {
    auto it = list.find(t);
    if (it == list.end()) return NULL;

    if (it->second)
      return it->second->create(data);
    return NULL;
  }

private:
  std::map<std::string, TypeItem<T> *> list;
};


class ActivityBase
{
public:
  ActivityBase(Scene* pScene):m_pScene(pScene) {}
  virtual ~ActivityBase();

  virtual void onAddNpc(SceneNpc* pNpc);
  virtual void onDelNpc(SceneNpc* pNpc);
  virtual bool onStart(DWORD curSec);
  virtual bool onProcess(DWORD curSec);
  virtual bool onStop(DWORD curSec);
  virtual bool onEnter(SceneUser* pUser) { return true; }
  virtual bool onLeave(SceneUser* pUser) { return true; }

  bool init(const SActivityCFG* pCfg);
  void setStartTime(DWORD startTime) { m_startTime = startTime; }
  DWORD getId() { return m_dwId; }
  void setUid(QWORD uid) { m_uid = uid; }
  QWORD getUid() { return m_uid; }
  inline bool isNeedDel();
  inline bool isExpire() { return m_isExpire; }
  Scene* getScene() { return m_pScene;  }
  bool run(DWORD curSec);
  bool checkState(DWORD curSec);
  bool onStateChange(DWORD curSec);
  DWORD getNpcCount(const TVecDWORD& vec);
  DWORD getNpcCount(DWORD npcId);
  void setSponsorName(std::string name) { m_sponsorName = name; }
  const std::string& getSponsorName() { return m_sponsorName; }

  bool stop();
  void onEnterScene(SceneUser* pUser);
  void onLeaveScene(SceneUser* pUser);
  //set
  void setMapId(DWORD mapId) { m_mapId = mapId; }
  void setProgress(DWORD progress) { m_progress = static_cast<EActProgress>(progress); }
  EActProgress getProgress() { return m_progress; }
  SceneNpc* getSceneNpc(DWORD id);    //
  
  void summonNpc();
  void pushNpc(NpcDefine def) { m_npcQueue.push_back(def); }
  void setKillerName(DWORD npcId, std::string killer);
  const std::string& getKillerName(DWORD npcId);
public:
  const TSetQWORD& getNpcGUIDS(DWORD npcid);
  bool checkHaveNpc(DWORD npcid);
  bool checkCondionStageOver(DWORD stage);
  void enableStage(DWORD stage);
  void onResetStage(DWORD stage);
public:
  void markRecordMonster(DWORD npcid) { m_setRecordMonsters.insert(npcid); }
  DWORD getKillCount(DWORD npcid);
  void resetKillCount(DWORD npcid);
  void onNpcDie(SceneNpc* pNpc);
public:
  void addSpecialEffect(DWORD dramaid, DWORD starttime);
  void delSpecialEffect(DWORD dramaid);
  void setActProgressNtfCmd(ActProgressNtfCmd& cmd);
  ActivityStageBase* getCurStage() { return m_pCurStage; }
protected:
  bool m_isStop = false;
  DWORD m_dwId = 0;
  QWORD m_uid = 0;
  Scene* m_pScene = nullptr;
  DWORD m_mapId = 0;
  DWORD m_startTime = 0;
  DWORD m_endTime = 0;
  bool m_isExpire = false;
  DWORD m_durtaion = 0;
  EActivityState m_state = EActivityState_none;
  std::vector<ActivityStageBase*> m_stage;
  std::vector<ActivityStageBase*>m_stopStage;
  std::map<DWORD/*id*/, std::set<QWORD/*npc guid*/>> m_mapNpcs;
  Cmd::EActProgress m_progress = EACTPROGRESS_NONE;
  std::string m_sponsorName;

  std::list<NpcDefine> m_npcQueue;   //待招的npc
  std::map<DWORD/*id*/, std::string/*killer*/> m_mapKiller;

  TSetDWORD m_setRecordMonsters; // 
  std::map<DWORD, DWORD> m_mapKillMonsters; // map(npcid, kill_count)
  std::map<DWORD, DWORD> m_mapSpecialEffect; // map(dramaid, starttime)
  ActivityStageBase* m_pCurStage = nullptr;
  ActProgressNtfCmd m_cmdCache;
};

/*class ActivityBCat :public ActivityBase
{
public:
  ActivityBCat(Scene* pScene) :ActivityBase(pScene)
  {}
private:
  virtual bool onStart(DWORD curSec);
  virtual bool onProcess(DWORD curSec);
  virtual bool onStop(DWORD curSec);
};*/

class ActivityQuest :public ActivityBase
{
public:
  ActivityQuest(Scene* pScene);
  virtual ~ActivityQuest();

  virtual void onAddNpc(SceneNpc* pNpc);
  virtual void onDelNpc(SceneNpc* pNpc);

  bool query(SceneUser* pUser, Query& rev);
  bool answer(SceneUser* pUser, Answer cmd);
private:
  void clear();
  void refresh();
  void delAllNpc();
private:
  std::map<DWORD/*q*/, DWORD/*a*/> m_quests;   //question&answer
  std::map<QWORD/*user id*/, std::queue<DWORD/*q*/>> m_answerUser;
  std::set<QWORD> m_wrongAnswer;
  DWORD m_reaminReward = 0;
};

class ActivityMoneyCat : public ActivityBase
{
  public:
    ActivityMoneyCat(Scene* pScene);
    virtual ~ActivityMoneyCat();

    DWORD getMoney() { return m_dwCurMoney; }
    void setMoney(DWORD money) { m_dwCurMoney = money; }
    bool checkMoneyEnd();

    void addUserMoney(const string& name, DWORD money);
    string getMaxMoneyUser();
    DWORD getUserMoney(const string& name);
  private:
    DWORD m_dwCurMoney = 0;
    std::map<string, DWORD> m_mapName2Money;
};

//卡普拉活动场内
class ActivityCapra : public ActivityBase
{
public:
  ActivityCapra(Scene* pScene);
  virtual ~ActivityCapra();

public:
  virtual bool onEnter(SceneUser* pUser);
  virtual bool onLeave(SceneUser* pUser);
};

class ActivityManager : public xSingleton<ActivityManager>
{
  friend class xSingleton<ActivityManager>;
  private:
    ActivityManager();
  public:
    virtual ~ActivityManager();

    void init();
    bool preProcess(const Cmd::ActivityTestAndSetSessionCmd& rev);
    void startActivity(const Cmd::ActivityTestAndSetSessionCmd& rev);
    ActivityBase* createActivity(const Cmd::ActivityTestAndSetSessionCmd& rev);
    ActivityBase* getActivityByUid(QWORD id);
    void stopActivity(DWORD id);
    void timer(DWORD curTime);
    void onEnterScene(SceneUser* pUser);
    void onLeaveScene(SceneUser* pUser);
    void onNpcDie(SceneNpc* pNpc);

    bool addGlobalActivity(DWORD id);
    bool delGlobalActivity(DWORD id);
    bool isOpen(DWORD id, bool isGlobal = true);

    bool addActivity(DWORD id);
    bool delActivity(DWORD id);
    void syncActivityToSession(DWORD id, bool open);

  private:
    std::map<QWORD/*uid*/, ActivityBase*> m_mapActvity;
    std::set<DWORD> m_setGlobalActivity;
    std::set<DWORD> m_setActivity;
};

