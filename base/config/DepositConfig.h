#pragma once

#include "xSingleton.h"
#include "xLuaTable.h"
#include "SceneItem.pb.h"
#include "SceneManual.pb.h"
#include "MiscConfig.h"
#include "ProtoCommon.pb.h"
#include <map>
using namespace Cmd;


struct SDepositCard 
{
  DWORD itemId;
  EDepositCardType type;
  DWORD year;
  DWORD month;
  DWORD headdress;    //头饰id
  SDepositTypeCFG typeCfg;
};
typedef map<DWORD, SDepositCard> TMapDepositCard;


enum EFuncType 
{
  EFuncType_NONE = 0,           // 空
  EFuncType_ADDCITTIME = 1,     // 防沉迷时间
  EFuncType_EXP = 2,            // 经验
  EFuncType_GOCITY = 3,         // 卡普拉公司的传送可以送你去任何地方
  EFuncType_JOBEXP = 4,         // job经验
  EFuncType_GUILDHONOR = 5,     // 荣誉之证
  EFuncType_PETWORK = 6,        // 宠物打工
  EFuncType_NORMALDROP = 7,     // 野外普通掉率和经验提升
};

enum EFunOper
{
  EFunOper_NONE = 0,
  EFunOper_ADD = 1,
  EFunOper_MULTIPLY = 2,
  EFunOper_OPEN = 3,
};

struct SDepositFunc
{
  EFuncType type;
  EFunOper oper;
  DWORD id;
  xLuaData param;
  string desc;
  DWORD msgid;
  template<typename T>
  T doFunc(T in)
  {
    T ret = in;
    switch (oper)
    {
    case EFunOper_ADD:
      ret = in + param.getTableInt("1");
      break;
    case EFunOper_MULTIPLY:
      ret = in * param.getTableInt("1") / 100;      //waring:此处参数以百分制
      break;

    default:
      break;
    }

    return ret;
  }
};
typedef map<DWORD, SDepositFunc> TMapDepositFunc;

enum ELimitType
{
  ELimitType_Min = 0,
  ELimitType_Char = 1,          //角色限购
  ELimitType_Account = 2,       //账号限购
  ELimitType_Branch = 3,        //版本限购
  ELimitType_ActivityOpen = 4,  //活动才开启
  ELimitType_Max = 5,
};

struct SDeposit
{
  friend class DepositConfig;
  DWORD id;
  EDepositCardType type;
  string productId;
  float rmb = 0;
  DWORD cardVersion = 0; //版本卡版本
  DWORD count = 0;
  DWORD count2 = 0;
  DWORD mailId = 0;
  DWORD monthLimit = 0;
  DWORD count3 = 0;     //warning 
  DWORD npcId = 0;
  DWORD duration = 0;
  ELimitType limitType = ELimitType_Min;
  string strDbColumn;          //数据库字段名
  DWORD dwActivityDiscount = 0; //是否是活动折扣商品
  QWORD quota = 0;             //购买增加的额度
  DWORD virginTag = 0;         //首充标记
  DWORD virginCount = 0;       //首充返利
  
  DWORD getTotalCount(bool bVirgin = false) const { return bVirgin ? count + virginCount : count + count2;}
  bool canBuy(DWORD regionId, QWORD accid) const;
  bool canBuy(DWORD regionId, QWORD accid, string &outKey, DWORD &outCount) const;
  DWORD getFromId() const;  //打折前的id
  DWORD getToId() const;    //打折后的id
  DWORD getItemId() const ;
private:
  DWORD itemId = 0;
};
typedef map<DWORD, SDeposit> TMapDeposit;


// config
class DepositConfig : public xSingleton<DepositConfig>
{
  friend class xSingleton<DepositConfig>;
  private:
    DepositConfig();
  public:
    virtual ~DepositConfig();

    bool loadConfig();
    bool checkConfig();

    void getVirginList(std::map<DWORD, DWORD>& map_virgin);

    const SDeposit* getSDeposit(DWORD id)
    {
      auto it = m_mapDepositCFG.find(id);
      if (it == m_mapDepositCFG.end()) return nullptr;
      return &(it->second);
    }

    const SDeposit* getSDeposit(string productId)
    {
      for (auto & v : m_mapDepositCFG)
      {
        if (v.second.productId == productId)
          return &(v.second);
      }
      return nullptr;
    }

    const SDepositCard* getSDepositCard(DWORD itemId)
    {
      auto it = m_mapCardCFG.find(itemId);
      if (it == m_mapCardCFG.end()) return nullptr;
      return &(it->second);
    }    

    SDepositFunc* getSDepositFunc(DWORD id)
    {
      auto it = m_mapFuncCFG.find(id);
      if (it == m_mapFuncCFG.end()) return nullptr;
      return &(it->second);
    }

    template<typename T>
    T doFunc(EDepositCardType cardType, EFuncType funcType, T in)
    {
        T ret = in;
        const SDepositTypeCFG* pCfg = MiscConfig::getMe().getDepositTypeCFG(cardType);
        if (!pCfg)
          return false;

        for (auto &v : pCfg->vecFuns)
        {
          SDepositFunc*pFunc = getSDepositFunc(v);
          if (!pFunc)
            continue;
          if (pFunc->type == funcType)
          {
            ret = pFunc->doFunc(ret);
          }
        }
        return ret;
    }
    bool hasFunc(EDepositCardType cardType, EFuncType funcType);
    const TMapDepositCard& getCardCFGList() const { return m_mapCardCFG; }
    const SDepositCard* getDepositCard(EDepositCardType type, DWORD year, DWORD month) const;

    template<class T> void loadDb(T func)
    {
      for (auto it = m_mapName2Id.begin(); it != m_mapName2Id.end(); ++it)
      {
        func(it->first, it->second);
      }
    }
  private:
    bool loadDepositConfig();
    bool loadDepositCardConfig();
    bool loadDepositFuncConfig();

  private:
    TMapDeposit m_mapDepositCFG;
    std::map<string/*dbcolumn name*/, DWORD/*dataid*/> m_mapName2Id;
    TMapDepositCard m_mapCardCFG;
    TMapDepositFunc m_mapFuncCFG;

  public:
    void getDepositMsg(TVecDWORD& vec)  // 获取月卡msg
    {
      for(auto& m : m_mapFuncCFG)
      {
        if(m.second.msgid)
          vec.push_back(m.second.msgid);
      }
    }
};

