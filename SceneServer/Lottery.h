#pragma once

#include "xDefine.h"
#include "SceneItem.pb.h"
#include "SessionCmd.pb.h"
#include "Var.pb.h"
#include "SceneUser2.pb.h"
#include "RecordCmd.pb.h"

class SLotteryCFG;
class SceneUser;

class Lottery
{
  public:
    Lottery(SceneUser* pUser);
    ~Lottery();
    void sendLotteryCfg(Cmd::ELotteryType type);
    bool lottery(Cmd::LotteryCmd& cmd);
    bool lotteryRecovery(Cmd::LotteryRecoveryCmd& cmd);
    bool isSkipLottery() { return m_bSkipLotteryAnim; }
    DWORD getPrice(const SLotteryCFG* pCfg, bool bDiscount);
    DWORD getDisCount(const SLotteryCFG* pCfg, bool bDiscount);
    Cmd::EAccVarType getAccVarType(DWORD type);
    static bool isLotterySource(Cmd::ESource source) 
    {
      return source == Cmd::ESOURCE_LOTTERY ||
        source == Cmd::ESOURCE_LOTTERY_HEAD ||
        source == Cmd::ESOURCE_LOTTERY_EQUIP ||
        source == Cmd::ESOURCE_LOTTERY_CARD ||
        source == Cmd::ESOURCE_LOTTERY_HEAD_OLD ||
        source == Cmd::ESOURCE_LOTTERY_CATLITTERBOX||
        source == Cmd::ESOURCE_LOTTERY_MAGIC;
    }
    //非券使用的才计数
    bool isNeedAddCount(bool useticket, Cmd::ELotteryType type) { return useticket == false && getAccVarType(type) != Cmd::EACCVARTYPE_MIN; }
    void getSpringReward(Cmd::ELotteryType type);
    bool lotteryGive(const Cmd::LotteryGiveInfo& info);
    void sendBuyLotteryCnt();
    Cmd::EVarType getVarCntType(Cmd::ELotteryType type);
    static Cmd::EOptionType getOptionType(Cmd::ELotteryType type);

    bool load(const Cmd::BlobLottery& oData);
    bool save(Cmd::BlobLottery* pData);

    bool loadAcc(const Cmd::BlobLottery& oData);
    bool saveAcc(Cmd::BlobLottery* pData);
 private:
    bool isRightTime(DWORD dwYear, DWORD dwMonth);
    bool fillCmd(Cmd::LotteryCmd& cmd);
    bool checkLotteryCnt(Cmd::ELotteryType type);
    DWORD getLotteryPoolCoin(DWORD dwType, DWORD dwYear, DWORD dwMonth);
    void addLotteryPoolCoin(DWORD dwType, DWORD dwYear, DWORD dwMonth, DWORD dwCoin);
 private:
    SceneUser* m_pUser = nullptr; 
    bool m_bSkipLotteryAnim = false;
    DWORD m_dwPoolTime = 0; // lottery pool update timestamp
    TVecDWORD m_vecItemPool;
    std::map<DWORD, DWORD> m_mapPoolCoin; // <key=type*1000000+year*100+month, coin>
};
