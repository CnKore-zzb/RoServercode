#pragma once
#include "xCommand.h"
#include "UserData.h"
#pragma pack(1)
struct UserRegInfo
{
  UserRegInfo()
  {
    accid = 0;
    zoneid = 0;
    bzero(acc, sizeof(acc));
  }
  const UserRegInfo& operator=(const UserRegInfo &u)
  {
    bcopy(&u, this, sizeof(*this));
    return *this;
  }
  UINT accid;
  UINT zoneid;
  char acc[MAX_NAMESIZE];
};

struct RegCmd : public xCommand
{
  RegCmd(unsigned char p):xCommand(REG_CMD, p){}
};

/****************************************************/
/****************  区验证消息开始  ******************/
/****************************************************/

//super->reg
const BYTE REQ_ZONE_REGCMD = 1;
struct ReqZoneRegCmd : public RegCmd 
{
  ReqZoneRegCmd():RegCmd(REQ_ZONE_REGCMD)
  {
    bzero(name, sizeof(name));
    id = 0;
    isKuaqu = false;
    bigVersion = 0;
    smallVersion = 0;
    openTime = 0;
    mergeZoneNum = 0;
  }
  char name[MAX_NAMESIZE];
  DWORD id;
  bool isKuaqu;
  DWORD bigVersion;
  DWORD smallVersion;
  DWORD openTime;
  DWORD mergeZoneNum;
  UINT zones[0];
};

//reg->super->session
const BYTE RET_ZONE_REGCMD = 2;
struct RetZoneRegCmd : public RegCmd
{
  RetZoneRegCmd():RegCmd(RET_ZONE_REGCMD)
  {
    bzero(info, sizeof(info));
  }
  unsigned char ret = 0;
  char info[MAX_NAMESIZE];
  DWORD ip = 0;
};

/****************************************************/
/****************  区验证消息结束  ******************/
/****************************************************/

/****************************************************/
/*************** 角色验证消息开始  ******************/
/****************************************************/
//super->reg
const BYTE VERTIFY_ERR_USER_REGCMD = 101;
struct VertifyErrUserRegCmd : public RegCmd 
{
  VertifyErrUserRegCmd():RegCmd(VERTIFY_ERR_USER_REGCMD)
  {
    bzero(info, sizeof(info));
  }
  UserRegInfo regInfo;
  char info[MAX_NAMESIZE];
};

#define GET_ADDR_REGCMD 102
struct GetAddrRegCmd : public RegCmd
{
  GetAddrRegCmd():RegCmd(GET_ADDR_REGCMD)
  {
  }
  UserRegInfo regInfo;
};

#define RET_ADDR_REGCMD 103
struct RetAddrRegCmd : public RegCmd
{
  RetAddrRegCmd():RegCmd(RET_ADDR_REGCMD)
  {
    ip = 0;
    port = 0;
  }
  UserRegInfo regInfo;
  DWORD ip;
  int port;
};

//请求登录
#define LOGIN_REGCMD 108
struct LoginRegCmd : public RegCmd
{
  QWORD id;
  char name[MAX_NAMESIZE];
  QWORD accid;
  DWORD ip;
  DWORD platformID;
  DWORD zoneID;
  char gateName[MAX_NAMESIZE];
  char deviceid[MAX_NAMESIZE];
  char phone[MAX_NAMESIZE];
  bool ignorepwd;
  char password[MAX_NAMESIZE];
  DWORD resettime;
  bool safeDevice;
  DWORD language;
  bool realAuthorized;
  DWORD maxbaselv;
  LoginRegCmd():RegCmd(LOGIN_REGCMD)
  {
    id = 0;
    bzero(name, sizeof(name));
    accid = 0;
    ip = 0;
    platformID = 0;
    zoneID = 0;
    bzero(gateName, sizeof(gateName));
    bzero(deviceid, sizeof(deviceid));
    bzero(phone, sizeof(phone));
    ignorepwd = false;
    bzero(password, sizeof(password));
    resettime = 0;
    safeDevice = false;
    language = 0;
    realAuthorized = false;
    maxbaselv = 0;
  }
};

enum
{
  UNREG_TYPE_QUIT,
  UNREG_TYPE_SELECT,
};

// 大退 发到Session处理
#define LOGIN_OUT_REGCMD 109
struct LoginOutRegCmd : public RegCmd
{
  QWORD accid;
  QWORD id;
  char gateName[MAX_NAMESIZE];
  bool overTime;
  bool deletechar = false;
  bool kick = false;   // 是否proxy踢出
  LoginOutRegCmd() : RegCmd(LOGIN_OUT_REGCMD)
  {
    accid = 0;
    id = 0;
    bzero(gateName, sizeof(gateName));
    overTime = false;
    deletechar = false;
  }
};


// 退出 Session通知场景退出
// Session->[Scene]->[Record]->Session
#define LOGIN_OUT_SCENE_REGCMD 110
struct LoginOutSceneRegCmd : public RegCmd
{
  QWORD accid = 0;
  QWORD charid;
  bool bDelete = false;
  LoginOutSceneRegCmd() : RegCmd(LOGIN_OUT_SCENE_REGCMD)
  {
    charid = 0;
    bDelete = false;
  }
};

// 通知网关退出
#define LOGIN_OUT_GATE_REGCMD 111
struct LoginOutGateRegCmd : public RegCmd
{
  QWORD accid;
  LoginOutGateRegCmd() : RegCmd(LOGIN_OUT_GATE_REGCMD)
  {
    accid = 0;
  }
};

//返回选择界面
#define SELECT_REGCMD 112
struct SelectRegCmd : public RegCmd
{
  QWORD accid;
  DWORD zoneID;
  SelectRegCmd():RegCmd(SELECT_REGCMD)
  {
    accid = 0; 
    zoneID = 0;
  }
};

//创建角色
#define CREATE_CHAR_REGCMD 113
struct CreateCharRegCmd : public RegCmd
{
  QWORD accid;
  DWORD zoneID;
  char name[MAX_NAMESIZE];
  WORD role_sex;
  WORD role_headPicIndex;
  WORD role_hairtype;
  WORD role_haircolor;
  WORD role_clothcolor;
  WORD role_career;
  WORD addict;
  char source[USER_SOURCE_LEN];
  char accName[ACC_NAME_LEN];
  DWORD ip;
  DWORD port;
  DWORD accCreateTime;
  DWORD firstZoneID;
  char gateName[MAX_NAMESIZE];
  DWORD sequence = 0;
  CreateCharRegCmd():RegCmd(CREATE_CHAR_REGCMD)
  {
    accid = 0;
    zoneID = 0;
    bzero(name, sizeof(name));
    role_sex = 0;
    role_headPicIndex = 0;
    role_hairtype = 0;
    role_haircolor = 0;
    role_clothcolor = 0;
    role_career = 0;
    addict = 1;
    bzero(source, sizeof(source));
    bzero(accName, sizeof(accName));
    ip = 0;
    port = 0;
    accCreateTime = 0;
    firstZoneID = 0;
  }
};

//删除角色
#define DELETE_CHAR_REGCMD 114
struct DelCharRegCmd:  public RegCmd
{
  QWORD id;
  QWORD accid;
  DelCharRegCmd():RegCmd(DELETE_CHAR_REGCMD)
  {
    id = 0;
    accid=0;
  }
};

//返回删除角色
#define RET_DELETE_CHAR_REGCMD 115
struct RetDeleteCharRegCmd:  public RegCmd
{
  QWORD id;
  QWORD accid;
  BYTE ret = 0;
  RetDeleteCharRegCmd():RegCmd(RET_DELETE_CHAR_REGCMD)
  {
    id = 0;
    accid = 0;
  }
};

//检查重名
#define CHECK_NAME_CHAR_REGCMD 116
struct CheckNameCharRegCmd:  public RegCmd
{
  QWORD accid;
  DWORD zoneID;
  char name[MAX_NAMESIZE];
  CheckNameCharRegCmd():RegCmd(CHECK_NAME_CHAR_REGCMD)
  {
    accid = 0;
    zoneID = 0;
    bzero(name, sizeof(name));
  }
};

//检查重名结果
#define RET_CHECK_NAME_CHAR_REGCMD 117
struct RetCheckNameCharRegCmd:  public RegCmd
{
  QWORD accid;
  DWORD zoneID;
  char name[MAX_NAMESIZE];
  BYTE ret;
  RetCheckNameCharRegCmd():RegCmd(RET_CHECK_NAME_CHAR_REGCMD)
  {
    accid = 0;
    zoneID = 0;
    bzero(name, sizeof(name));
    ret = 0;
  }
};

//创建角色结果
#define RET_CREATE_CHAR_REGCMD 118
struct RetCreateCharRegCmd : public RegCmd
{
  QWORD accid;
  DWORD zoneID;
  char name[MAX_NAMESIZE];
  BYTE ret;
  DWORD ip;
  DWORD port;
  RetCreateCharRegCmd():RegCmd(RET_CREATE_CHAR_REGCMD)
  {
    accid = 0;
    zoneID = 0;
    bzero(name, sizeof(name));
    ret = 0;
    ip = 0;
    port = 0;
  }
};

//检查角色是否登录 GATE->SESSION
#define CHECK_LOGIN_REGCMD 119
struct CheckLoginRegCmd : public RegCmd
{
  QWORD accid;
  WORD zoneID;
  char gateName[MAX_NAMESIZE];
  CheckLoginRegCmd():RegCmd(CHECK_LOGIN_REGCMD)
  {
    accid = 0;
    zoneID = 0;
    bzero(gateName, sizeof(gateName));
  }
};

//返回角色是否登录 SESSION->GATE
#define RET_CHECK_LOGIN_REGCMD 120
struct RetCheckLoginRegCmd : public RegCmd
{
  QWORD accid;
  WORD zoneID;
  BYTE ret;
  RetCheckLoginRegCmd():RegCmd(RET_CHECK_LOGIN_REGCMD)
  {
    accid = 0;
    zoneID = 0;
    ret = 0;
  }
};

//场景登录成功前客户端退出,GATE->SESSION
#define QUIT_LOGIN_REGCMD 121
struct QuitLoginRegCmd : public RegCmd
{
  QWORD accid;
  DWORD zoneID;
  char gateName[MAX_NAMESIZE];
  QuitLoginRegCmd():RegCmd(QUIT_LOGIN_REGCMD)
  {
    accid=0;
    zoneID = 0;
    bzero(gateName, sizeof(gateName));
  }
};

//将已登录玩家踢下线 gate -> session -> record
#define KICK_PRIOR_REGUSER 123
struct KickPriorUserRegCmd : public RegCmd
{
  QWORD id;
  QWORD accid;
  DWORD zoneID;
  char gateName[MAX_NAMESIZE];
  KickPriorUserRegCmd() : RegCmd(KICK_PRIOR_REGUSER)
  {
    id = 0;
    accid = 0;
    zoneID = 0;
    bzero(gateName, sizeof(gateName));
  }
};

//切换线程处理玩家下线
#define USER_BINARY_LOGOUT_REGUSER 125
struct UserBinaryLogoutRegCmd : public RegCmd
{
  QWORD charid;
  UserBinaryLogoutRegCmd() : RegCmd(USER_BINARY_LOGOUT_REGUSER)
  {
    charid = 0;
  }
};

//切换线程删除玩家管理中的玩家
#define MUTLI_THREAD_DELUSER_REGUSER 126
struct MultiThreadDelUserRegCmd : public RegCmd
{
  QWORD charid;
  MultiThreadDelUserRegCmd() : RegCmd(MUTLI_THREAD_DELUSER_REGUSER)
  {
    charid = 0;
  }
};

//创建角色成功
#define CREATE_OK_CHAR_REGCMD 127
struct CreateOKCharRegCmd : public RegCmd
{
  QWORD accid = 0;
  QWORD charid = 0;
  char gatename[MAX_NAMESIZE];
  CreateOKCharRegCmd():RegCmd(CREATE_OK_CHAR_REGCMD)
  {
    bzero(gatename, sizeof(gatename));
  }
};

// 切换区
#define CHANGE_ZONE_CHAR_REGCMD 128
struct ChangeZoneCharRegCmd : public RegCmd
{
  QWORD charid = 0;
  ChangeZoneCharRegCmd() : RegCmd(CHANGE_ZONE_CHAR_REGCMD)
  {
  }
};

/****************************************************/
/*************** 角色验证消息结束  ******************/
/****************************************************/

//客服请求登录
#define KEFU_LOGIN_REGCMD 130
struct KefuLoginRegCmd : public RegCmd
{
  char gateName[MAX_NAMESIZE];
  char kefuName[MAX_NAMESIZE]; //客服名
  QWORD kefuID;	//客服ID
  QWORD userID;	//服务的玩家ID
  KefuLoginRegCmd():RegCmd(KEFU_LOGIN_REGCMD)
  {
    bzero(gateName, sizeof(gateName));
    bzero(kefuName, sizeof(kefuName));
    kefuID = 0;
    userID = 0;
  }
};

//客服退出
#define KEFU_LOGOUT_REGCMD 131
struct KefuLogOutRegCmd : public RegCmd
{
  DWORD zoneID;
  QWORD kefuID;	//客服ID
  KefuLogOutRegCmd():RegCmd(KEFU_LOGOUT_REGCMD)
  {
    zoneID = 0;
    kefuID = 0;
  }
};

//角色改名刷新快照
#define USER_RENAME_REGCMD 132
struct UserRenameRegCmd : public RegCmd
{
  QWORD accid;
  UserRenameRegCmd():RegCmd(USER_RENAME_REGCMD)
  {
    accid = 0;
  }
};

#pragma pack()
