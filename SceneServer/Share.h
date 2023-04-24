#pragma once

#include "xDefine.h"
#include "xSingleton.h"
#include "TableStruct.h"
#include "RecordCmd.pb.h"
#include "ProtoCommon.pb.h"
#include <list>
#include <algorithm>

using namespace std;

#define MAX_COUNT 50

struct CmpByValue {   //从大到小排序
  bool operator()(const ShareCalcItem& lhs, const ShareCalcItem& rhs) {
    if (lhs.high_value() > rhs.high_value())
      return true;
    else if (lhs.high_value() == rhs.high_value())
    {
      if (lhs.low_value() > rhs.low_value())
        return true;
      else
        return false;
    }
    else
      return false;
  }
};

class SceneUser;

struct SShareCalc
{
  EShareDataType type ;
  std::map<QWORD/*key1<<32+key2*/, ShareCalcItem> items;

  void fromData(const Cmd::ShareCalc& protoData);
  void toData(Cmd::ShareCalc* pProtoData);
  void add(QWORD key, QWORD value);
  void convert64To32(QWORD in64, DWORD& h32, DWORD &l32);
  void convert32To64(DWORD h32, DWORD l32, QWORD &out64);

  vector<ShareCalcItem> vecMax;
};

inline void SShareCalc::convert64To32(QWORD in64, DWORD& h32, DWORD &l32)
{
  h32 = (DWORD)(in64 >> 32);
  l32 = (DWORD)in64;
}

inline void SShareCalc::convert32To64(DWORD h32, DWORD l32, QWORD &out64)
{
  out64 = h32;
  out64 = (out64 << 32) + l32;
}

class Share
{
public:
  Share(SceneUser* pUser);
  ~Share();

  bool load(const BlobShare& data);
  bool save(BlobShare *data);

  void addNormalData(EShareDataType eType, QWORD value);
  void addCalcData(EShareDataType eType, QWORD key, QWORD value);
  void onKillMvp(DWORD npcId, DWORD mapId);
  void onHand(QWORD otherId, DWORD mapId);
  void onCarrier(QWORD otherId, DWORD mapId);
  void onTradeBuy(DWORD itemId, QWORD totalPrice, DWORD refineLv);
  void onTradeSell(DWORD itemId, QWORD totalPrice, DWORD refineLv);
  void onHurt(const std::string& target, DWORD value);
  //开了神秘箱子
  void onOpenMysteryBox(DWORD itemId);

  DWORD getNormalData(EShareDataType eType) const;
  QWORD getMaxCount(EShareDataType eType);
  QWORD getMaxKey(EShareDataType eType);
  bool collectMostCharID(EShareDataType eType, DWORD dwNum, TSetQWORD& setCharIDs);
  bool collectShareCharID(EShareDataType eType, TSetQWORD& setCharIDs);
private:
  SceneUser* m_pUser = nullptr;
  std::map<EShareDataType, QWORD/*value*/> m_mapNomalData;
  std::map<EShareDataType, SShareCalc> m_mapCalcData;
  Cmd::ShareFirstMvp m_oFirstMvp;          //第一次mvp击杀情况
  string m_strFirstPhoto;
  Cmd::ShareFirstHand m_oFirstHand ;       //第一次牵手
  Cmd::ShareFirstCarrier m_oFirstCarrier;  //第一次摩天轮
  Cmd::ShareTrade m_oMaxTradeBuy;          //交易所最高一笔购买
  Cmd::ShareTrade m_oMaxTradeSell;         //交易所最高一次出售
  Cmd::ShareDamage m_oMaxDamage;           //打出过的最高一次的输出 
  TSetDWORD m_setMysteryBox;                //神秘箱子开出的物品id
};
