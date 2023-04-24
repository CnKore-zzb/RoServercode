#pragma once

#include "xNoncopyable.h"
#include "GuildSCmd.pb.h"
#include "xDefine.h"
#include "xTime.h"

// data version, 新创建的公会不执行
const DWORD GVG_DATA_VERSION = 1;

using namespace std;
using namespace Cmd;
class Guild;
class GuildGvg : private xNoncopyable
{
  public:
    GuildGvg(Guild* pGuild);
    virtual ~GuildGvg();

    bool fromData(const BlobGGvg& data);
    bool toData(BlobGGvg* pData);
    void timer(DWORD curTime);
  public:
    void addGvgPartinUser(QWORD userid);
    const TSetQWORD& getGvgPartInUsers();

    void joinSuperGvg(DWORD begintime);
    bool inSuperGvg() const { return m_dwSuperGvgTime && m_dwSuperGvgTime > now(); }
    void endSuperGvg();
    void finishSuperGvg(DWORD rank);
    DWORD getSuperGvgLv() const;

    /*强制设置胜率信息, 测试使用*/
    void setFireCntAndScore(DWORD cnt, DWORD score);
    void checkVersion();
  private:
    void version_1();
  private:
    Guild* m_pGuild = nullptr;
    pair<DWORD, TSetQWORD> m_oPartinTime2Users;
    DWORD m_dwSuperBeginTime = 0; /*开始时间, 不保存数据库*/
    DWORD m_dwSuperGvgTime = 0;/*参加gvg决战的结束时间, 结束前不可攻击水晶*/
    DWORD m_dwSuperGvgCnt = 0; /*参战次数*/
    DWORD m_dwSuperGvgScore = 0; /*决战总积分*/
    DWORD m_dwVersion = 0;
};
