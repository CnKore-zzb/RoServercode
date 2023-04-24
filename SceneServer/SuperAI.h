#pragma once

#include "SuperAIConfig.h"
#include "AICondition.h"
#include "AIAction.h"

// super ai manager
class SuperAIManager : public xSingleton<SuperAIManager>
{
  friend class xSingleton<SuperAIManager>;
  private:
    SuperAIManager();
  public:
    virtual ~SuperAIManager();

    bool init();
  private:
    void regAIItem();
};

// aiitem
template <typename B>
struct AIItem
{
  public:
    std::string m_strName;
    AIItem() {}
    virtual ~AIItem() {}
    virtual B* create(const xLuaData &data) = 0;
};

// aicreator
template <typename B, typename T>
class AIItemCreator : public AIItem<B>
{
  using AIItem<B>::m_strName;
  public:
  AIItemCreator(const char *s):AIItem<B>()
  {
    m_strName = s;
  }
  B* create(const xLuaData &data)
  {
    T *pT = NEW T(data);
    pT->setName(m_strName);
    // XLOG("[ai],创建,%s", m_strName.c_str());
    return pT;
  }
};

// aiitemfactory
template <typename T>
class AIItemFactory : public xSingleton<AIItemFactory<T> >
{
  friend class xSingleton<AIItemFactory<T> >;

  public:
    ~AIItemFactory()
    {
      final();
    }
    void final()
    {
      for (auto it=list.begin(); it!=list.end(); ++it)
        SAFE_DELETE(it->second);
      list.clear();
    }
    void reg(AIItem<T> *item)
    {
      if (!item) return;

      list.insert(std::make_pair(item->m_strName, item));
    }

    T* create(const string& t, const xLuaData &data)
    {
      auto it = list.find(t);
      if (it==list.end()) return NULL;

      if (it->second)
        return it->second->create(data);
      return NULL;
    }

  private:
    std::map<std::string, AIItem<T> *> list;
};

// super ai item
typedef vector<AITarget*> TVecAITarget;
typedef vector<AIAction*> TVecAIAction;
class SuperAIItem
{
  friend class SuperAI;
  public:
    SuperAIItem();
    ~SuperAIItem();

    const string& getConditionName() const { return m_pCondition->getName(); }

    void reset();
    bool canTrigger(SceneNpc* pNpc) const;
    AI_RET_ENUM exe(SceneNpc* pNpc) const;
    DWORD getWeight() const;

  private:
    AICondition* m_pCondition = nullptr;

    TVecAITarget m_vecTargets;
    TVecAIAction m_vecActions;
    DWORD m_dwID = 0;
    bool m_bActive = false;
};
typedef vector<SuperAIItem*> TVecSuperAIItem;

// super ai
class SceneNpc;
class SuperAI
{
  public:
    SuperAI(SceneNpc* pNpc);
    ~SuperAI();

    bool init(const TSetDWORD& setIDs);
    void checkSig(string name);    //检查触发
    bool changeAI(const TSetDWORD& setIDs);   //改变ai

  private:
    void clear();

  private:
    SceneNpc* m_pNpc = nullptr;
    TVecSuperAIItem m_vecAIItems;

  private:
    void clearTVecSuperAIItem(TVecSuperAIItem &vec);

  public:
    void setFunctionID(DWORD id) { m_dwFunctionID = id; }
    DWORD getFunctionID() { return m_dwFunctionID; }

    bool addAI(DWORD id);
  private:
    // AI检测变量, 触发AI
    DWORD m_dwFunctionID = 0;
};

