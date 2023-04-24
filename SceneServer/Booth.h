#pragma once

#include "xDefine.h"
#include "RecordCmd.pb.h"
#include "SceneUser2.pb.h"

class SceneUser;
class Booth
{
  public:
    Booth(SceneUser* pUser);
    ~Booth();

  public:
    bool load(const Cmd::BlobBooth& oData);
    bool save(Cmd::BlobBooth* pData);
    bool toBoothData(Cmd::BoothInfo* pData);

  public:
    bool req(const Cmd::BoothReqUserCmd& cmd);
    bool open(const std::string& name);
    bool update(const std::string& name);
    bool close();

  public:
    void onLogin();
    void onLeaveScene();
    void onRelive(Cmd::EReliveType eType);
    void onCarrier();

  public:
    DWORD getScore() { return m_dwScore; }
    void addScore(DWORD score);

    DWORD getSize();
    bool hasOpen() { return m_bOpen; }
    std::string getName() { return m_strName; }
    Cmd::EBoothSign getSign() { return m_eSign; }

  public:
    void calcTax(QWORD tax);

  private:
    void sendCmdToMe(Cmd::EBoothOper oper);
    void sendCmdToNine(Cmd::EBoothOper oper);
    void sendCmdToMatchServer(Cmd::EBoothOper oper);

  private:
    SceneUser* m_pUser = nullptr;
    DWORD m_dwScore; // 摊位积分
    bool m_bOpen; // 是否开业
    std::string m_strName; // 摊位名字
    Cmd::EBoothSign m_eSign; // 摊位招牌
    DWORD m_dwUpdateTime; // 改名最小时间
};
