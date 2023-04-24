#pragma once

#include "xDefine.h"

#pragma pack(1)

enum USER_STATE
{
  USER_STATE_NONE = 0,
  USER_STATE_REG,
  USER_STATE_SELECT,
  USER_STATE_LOGIN,
  USER_STATE_RUN,
  USER_STATE_CHANGE_SCENE,
  USER_STATE_QUIT
};

struct SnapShotData
{
  QWORD id;
  char name[MAX_NAMESIZE];
  DWORD mapID;
  DWORD zoneid;
  DWORD originalzoneid;
  DWORD level;
  DWORD male;//性别
  DWORD type;//职业
  DWORD hair;//头发
  DWORD haircolor;//衣服
  DWORD lefthand;
  DWORD righthand;
  DWORD body;
  DWORD head;
  DWORD back;
  DWORD face;
  DWORD tail;
  DWORD mount;
  DWORD title;
  DWORD eye;
  DWORD partnerid;
  DWORD portrait;
  DWORD mouth;
  DWORD clothcolor = 0;
  DWORD m_dwNoLoginTime = 0;
  DWORD m_dwSequence = 0;
  DWORD m_dwOpen = 0;
  SnapShotData()
  {
    bzero(this, sizeof(*this));
  }
  void clean()
  {
    bzero(this, sizeof(*this));
  }
};

struct AccBaseData
{
  QWORD m_qwAccID = 0;
  QWORD m_qwCharID = 0;       // 当前登录的角色id 无登录角色为0
  DWORD m_dwFourthChar = 0;
  DWORD m_dwFifthChar = 0;
  QWORD m_qwLastSelect = 0;

  QWORD m_qwDeleteCharID = 0;   // 要删除的角色id
  DWORD m_dwDeleteTime = 0;     // 实际删除的时间
  DWORD m_dwLastDeleteTime = 0; // 上次删除角色的时间
  DWORD m_dwDeleteCount = 0;    // 今天删除的次数
  DWORD m_dwDeleteCountDays = 0;    // 今天删除的次数的天计数

  DWORD m_dwPwdResetTime = 0;     //安全密码重置到期时间
  char m_strPassword[LEN_256];     //安全密码
  DWORD m_dwNoLoginTime = 0;        //封号截止时间
  QWORD m_qwMainCharId = 0;      //主号（职业补偿用）
  AccBaseData()
  {
    bzero(this, sizeof(*this));
  }
};

//GM权限
enum HumanPrivilege
{
  HUMAN_NORMAL = 1,
  HUMAN_GM = 2,
  HUMAN_GM_CAPTAIN = 4,
  HUMAN_SUPER_GM = 8,
  HUMAN_TEST = 16,
};

struct GateUserData
{
  char sceneServerName[MAX_NAMESIZE];
  QWORD charid;
  char name[MAX_NAMESIZE];
  GateUserData()
  {
    bzero(this, sizeof(*this));
  }
};
//注销类型
enum class UnregType
{
  Null        = 0,
  Normal      = 1,        //正常下线
  ChangeScene = 2,        //跨服切换场景
  ServerStop  = 3,        //服务器停机
  Kuaqu       = 4,        //跨区
  Delete      = 5,        //删号
  SaveBinary  = 10000,    //只是保存二制数据，非下线
  GMSave = 10001,         //GM命令强制刷新数据库，非下线
};

#pragma pack()
