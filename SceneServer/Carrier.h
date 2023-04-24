#pragma once

#include "xNoncopyable.h"
#include "xDefine.h"
#include "xPos.h"

class SceneUser;

namespace Cmd
{
  class CarrierMoveUserCmd;
  class ReachCarrierUserCmd;
  class BlobCarrier;
};

struct CarrierData
{
  DWORD m_dwCarrierID = 0;    // 唯一id 标识Carrier类型
  DWORD m_dwLine = 0;         // 线路  客户端用
  DWORD m_dwAssembleID = 0;   // 装饰id  换装用
  DWORD m_dwNeedAnimation = 1;    // 是否需要过场动画, 1:需要， 0：不需要
  QWORD m_qwMasterID = 0;         // 载具主人id  已经登上载具
  xPos m_oStartPos;           // 起始坐标

  void reset()
  {
    m_dwCarrierID = m_dwLine = m_dwAssembleID = 0;
    m_dwNeedAnimation = 1;
    m_qwMasterID = 0;
    m_oStartPos.clear();
  }
};

class Carrier : private xNoncopyable
{
  public:
    Carrier(SceneUser *u);
    ~Carrier();

    // 自己创建载具
    bool create(DWORD carrierID, DWORD line, DWORD dwNeedAct = 1, DWORD invite=1);
    // 别人加入载具
    bool checkMasterCanBeJoin(QWORD charid);
    bool join(QWORD charid);
    bool checkUserCanJoinMe(SceneUser *user);
    bool join(SceneUser *user, bool nofity=true);
    void onUserEnter();
    // 离开载具
    void leave();
    // 到达
    void reach(Cmd::ReachCarrierUserCmd &);
    // 移动
    bool move(Cmd::CarrierMoveUserCmd &message);

    void offline();

    // 上载具
    void sendMeToNine();

    bool canJoin(SceneUser *user);
    bool isMaster(QWORD id);
    void clear();
    // 载具启动
    void start();

    // 换装饰
    bool changeAssemble(DWORD dwAssembleID);

    void save(Cmd::BlobCarrier *data);
    void load(const Cmd::BlobCarrier &data);
    void sendCmdAll(const void* data, unsigned short len);

    bool has() const { return m_oData.m_dwCarrierID || m_oData.m_qwMasterID; }

    void setCarrierID(DWORD dwID);
    DWORD getCarrierID() const { return m_oData.m_dwCarrierID; }

  public:
    // 公用字段
    SceneUser *m_pUser = NULL;

    // 载具数据 所有成员一致
    CarrierData m_oData;

    // 个人数据
    DWORD m_dwIndex = 0;        // 座位编号
    DWORD m_dwProgress = 0;     // 进度 客户端用

    // 需要保存的数据
    QWORD m_qwJoinMasterID = 0;   // 保存需要加入的载具玩家id
    DWORD m_dwBuyAssembleID = 0;  // 保存没有装饰 没有创建载具前

    // 主驾驶 不包括自己
    std::map<DWORD, SceneUser *> m_members;  // 座位编号 成员列表
    std::set<QWORD> m_oInvites;   // 邀请列表

    // 成员
    std::set<QWORD> m_oBeInvites;   // 邀请我的列表  还未登上载具
};
