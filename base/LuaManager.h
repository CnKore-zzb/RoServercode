/**
 * @file LuaManager.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2016-06-02
 */

#pragma once

#include "xDefine.h"
#include "xSingleton.h"
#include "lua/lua_tinker.h"
#include "xLuaTable.h"

#define CLASS_FUNC(klass,member) lua_tinker::class_def<klass>(_L, #member, &klass::member)
#define CLASS_ADD(klass)        lua_tinker::class_add<klass>(_L, #klass);
#define ENUM(name)          lua_tinker::set(_L, #name, name)
#define GLOBAL(name)        lua_tinker::def(_L, #name, name)

using std::string;
using std::vector;
using std::map;
using std::pair;

struct SLuaParams
{
  vector<float> m_vecParams;

  xLuaData m_oData;

  /*初始化变量个数需与ServerLua.lua : getParams(index) 对应, 当前为5个参数,修改需处理Lua调用*/
  void init(float a = 0, float b = 0, float c = 0, float d = 0, float e = 0)
  {
    m_vecParams.push_back(a);
    m_vecParams.push_back(b);
    m_vecParams.push_back(c);
    m_vecParams.push_back(d);
    m_vecParams.push_back(e);
  }
  void add(float params)
  {
    m_vecParams.push_back(params);
  }

  float getParams(DWORD index)
  {
    if (index == 0 || m_vecParams.size() == 0 || m_vecParams.size() < index)
      return 0.0f;
    return m_vecParams[index - 1];
  }

  void addKeyValue(const char* key, float value) { m_oData.setData(key, value); }
  void clear()
  {
    m_vecParams.clear();
    m_oData.clear();
  }

  SLuaParams() {}
};

struct SLuaSkillParam
{
  DWORD m_dwSkillID = 0;
  DWORD m_dwTargetNumAndIndex = 0;
  DWORD m_dwArrowID = 0;
  bool m_bShareDam = false;
  map<QWORD, pair<float, DWORD>> m_mapID2ShareDam;
  map<DWORD, DWORD> m_mapBuff2Layer;

  void addShareDam(QWORD charid, float value, DWORD type)
  {
    pair<float, DWORD>& daminfo = m_mapID2ShareDam[charid];
    daminfo.first = value;
    daminfo.second = type;
  }

  DWORD getSkillID() { return m_dwSkillID; }
  DWORD getTargetsNum() { return m_dwTargetNumAndIndex; }
  DWORD getArrowID() { return m_dwArrowID; }
  bool haveShareDam() { return m_bShareDam; }
  /* getBuffLayer return -1 表示无记录*/
  int getBuffLayer(DWORD id) { auto it = m_mapBuff2Layer.find(id); return it != m_mapBuff2Layer.end() ? it->second : -1;}
  SLuaSkillParam() {}
};

struct SLuaNumberArray
{
  TSetDWORD m_setDWORD;
  TSetQWORD m_setQWORD;

  // c 调用
  void clear()
  {
    m_setDWORD.clear();
    m_setQWORD.clear();
  }
  void setDWORD(const TSetDWORD& dset)
  {
    m_setDWORD.clear();
    m_setDWORD.insert(dset.begin(), dset.end());
  }
  void setQWORD(const TSetQWORD& qset)
  {
    m_setQWORD.clear();
    m_setQWORD.insert(qset.begin(), qset.end());
  }
  // lua 调用
  DWORD getDWArraySize() { return m_setDWORD.size(); }
  //DWORD getQWArraySize() { return m_setQWORD.size(); }
  DWORD getDWValueByIndex(DWORD index) // index begin from 1
  {
    DWORD i = 1;
    for (auto &s : m_setDWORD)
    {
      if (i == index)
        return s;
      i++;
    }
    return 0;
  }
};

class LuaManager : public xSingleton<LuaManager>
{
  friend class xSingleton;
  private:
    LuaManager();
  public:
    virtual ~LuaManager();

    bool load();
    bool reload();
    void setregister(std::function<void()> func) { registerfunc = func; }
    lua_State* getLuaState() const { return _L; }

    template<typename R>
    inline R call(const string& name)
      { return lua_tinker::call<R>(_L, name.c_str());}
    template<typename R, typename T1>
    inline R call(const std::string& name, const T1& t1)
      { return lua_tinker::call<R>(_L, name.c_str(), t1); }
    template<typename R, typename T1, typename T2>
    inline R call(const std::string& name, const T1& t1, const T2& t2)
      { return lua_tinker::call<R>(_L, name.c_str(), t1, t2); }
    template<typename R, typename T1, typename T2, typename T3>
    inline R call(const std::string& name, const T1& t1, const T2& t2, const T3& t3)
      { return lua_tinker::call<R>(_L, name.c_str(), t1, t2, t3); }
    template<typename R, typename T1, typename T2, typename T3, typename T4>
    inline R call(const std::string& name, const T1& t1, const T2& t2, const T3& t3, const T4& t4)
      { return lua_tinker::call<R>(_L, name.c_str(), t1, t2, t3, t4); }
    template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
    inline R call(const std::string& name, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5)
      { return lua_tinker::call<R>(_L, name.c_str(), t1, t2, t3, t4, t5); }
    template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    inline R call(const std::string& name, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6)
      { return lua_tinker::call<R>(_L, name.c_str(), t1, t2, t3, t4, t5, t6); }
    template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    inline R call(const std::string& name, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7)
      { return lua_tinker::call<R>(_L, name.c_str(), t1, t2, t3, t4, t5, t6, t7); }
    template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    inline R call(const std::string& name, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8)
      { return lua_tinker::call<R>(_L, name.c_str(), t1, t2, t3, t4, t5, t6, t7, t8); }

  private:
    void static cPlusLog(const char* str);
    void static cPlusLogError(const char* str);
    void static sendMail(QWORD qwTargetID, DWORD dwMailID);
    void static sendOptMail(QWORD qwTargetID, const char* sender, const char* title, const char* msg, DWORD dwItemID, DWORD dwCount);
    void static sendMsg(QWORD qwTargetID, DWORD dwMsgID);

    void registerinferface();
  private:
    lua_State * _L = nullptr;
    std::function<void()> registerfunc;
};

