#pragma once

#include "xCommand.h"
#include "UserData.h"
#pragma pack(1)

struct GatewayCmd : public xCommand
{
  GatewayCmd(unsigned char p=0): xCommand(GATEWAY_CMD, p){}
};

//Scene to Gate,网关数据
#define USER_DATA_GATEWAYCMD 1
struct UserDataGatewayCmd : public GatewayCmd
{
  QWORD accid;
  GateUserData data;
  UserDataGatewayCmd() : GatewayCmd(USER_DATA_GATEWAYCMD)
  {
    accid = 0;
  }
};

/*网关转发用户指令*/
#define FORWARD_USERCMD_GATEWAYCMD 2
struct ForwardUserCmdGatewayCmd : public GatewayCmd
{
  QWORD accid;
  WORD len;
  BYTE data[0];
  ForwardUserCmdGatewayCmd():GatewayCmd(FORWARD_USERCMD_GATEWAYCMD)
  {
    accid = 0;
    len = 0;
  }
};

/*转发用户指令到User*/
#define FORWARD_TO_USER_GATEWAY_CMD 3
struct ForwardToUserGatewayCmd : public GatewayCmd
{
  QWORD accid;
  WORD len;
  BYTE data[0];
  ForwardToUserGatewayCmd():GatewayCmd(FORWARD_TO_USER_GATEWAY_CMD)
  {
    accid = 0;
    len = 0;
  }
};

/*转发用户指令到LoginUser*/
#define FORWARD_TO_LOGIN_USER_GATEWAY_CMD 4
struct ForwardToLoginUserGatewayCmd : public GatewayCmd
{
  QWORD accid = 0;
  WORD len = 0;
  BYTE data[0];
  ForwardToLoginUserGatewayCmd():GatewayCmd(FORWARD_TO_LOGIN_USER_GATEWAY_CMD)
  {
  }
};

//转发全体消息
#define FORWARD_ALL_USER_GATEWAYCMD 5
struct ForwardAllUserGatewayCmd: public GatewayCmd
{
  WORD platformID;
  QWORD exclude;  //此人不发
  WORD len;
  BYTE data[0];
  ForwardAllUserGatewayCmd():GatewayCmd(FORWARD_ALL_USER_GATEWAYCMD)
  {
    platformID = 0;
    exclude=0;
    len = 0;
  }
};

//转发一级索引消息
#define BROADCAST_ONE_LEVEL_INDEX_GATEWAYCMD 6
struct BroadcastOneLevelIndexGatewayCmd: public GatewayCmd
{
  ONE_LEVEL_INDEX_TYPE indexT;
  QWORD i;
  WORD len;
  QWORD exclude;
  DWORD ip;
  BYTE data[0];
  BroadcastOneLevelIndexGatewayCmd():GatewayCmd(BROADCAST_ONE_LEVEL_INDEX_GATEWAYCMD)
  {
    indexT = ONE_LEVEL_INDEX_TYPE_MAX;
    i = 0;
    len = 0;
    ip = 0;
    exclude = 0;
  }
};

//添加一级索引
#define ADD_ONE_LEVEL_INDEX_GATEWAYCMD 7
struct AddOneLevelIndexGatewayCmd: public GatewayCmd
{
  ONE_LEVEL_INDEX_TYPE indexT;
  QWORD i;
  QWORD accid;
  AddOneLevelIndexGatewayCmd():GatewayCmd(ADD_ONE_LEVEL_INDEX_GATEWAYCMD)
  {
    i = 0;
    indexT = ONE_LEVEL_INDEX_TYPE_MAX;
    accid = 0;
  }
};

//删除一级索引
#define DEL_ONE_LEVEL_INDEX_GATEWAYCMD 8
struct DelOneLevelIndexGatewayCmd: public GatewayCmd
{
  ONE_LEVEL_INDEX_TYPE indexT;
  QWORD i;
  QWORD accid;
  DelOneLevelIndexGatewayCmd():GatewayCmd(DEL_ONE_LEVEL_INDEX_GATEWAYCMD)
  {
    i = 0;
    indexT = ONE_LEVEL_INDEX_TYPE_MAX;
    accid = 0;
  }
};

struct GateIndexFilter
{
  QWORD exclude = 0;
  WORD group = 0;
  QWORD team = 0;
  GateIndexFilter(){}
  GateIndexFilter(QWORD ex):exclude(ex) {}
  GateIndexFilter(QWORD t, WORD gr):group(gr),team(t) {}
};

//转发二级索引消息
#define BROADCAST_TWO_LEVEL_INDEX_GATEWAYCMD 9
struct BroadcastTwoLevelIndexGatewayCmd: public GatewayCmd
{
  DWORD i;
  BYTE num;
  WORD i2s[9];
  GateIndexFilter filter;
  WORD len;
  BYTE data[0];
  BroadcastTwoLevelIndexGatewayCmd():GatewayCmd(BROADCAST_TWO_LEVEL_INDEX_GATEWAYCMD)
  {
    i = 0;
    num = 0;
    bzero(i2s, sizeof(i2s));
    len = 0;
  }
};

//添加二级索引
#define ADD_TWO_LEVEL_INDEX_GATEWAYCMD 10
struct AddTwoLevelIndexGatewayCmd: public GatewayCmd
{
  DWORD i;
  WORD i2;
  QWORD accid;
  AddTwoLevelIndexGatewayCmd():GatewayCmd(ADD_TWO_LEVEL_INDEX_GATEWAYCMD)
  {
    i = 0;
    i2 = 0;
    accid = 0;
  }
};

//删除二级索引
#define DEL_TWO_LEVEL_INDEX_GATEWAYCMD 11
struct DelTwoLevelIndexGatewayCmd: public GatewayCmd
{
  DWORD i;
  WORD i2;
  QWORD accid;
  DelTwoLevelIndexGatewayCmd():GatewayCmd(DEL_TWO_LEVEL_INDEX_GATEWAYCMD)
  {
    i = 0;
    i2 = 0;
    accid = 0;
  }
};

//同步玩家等级
#define SYN_LEVEL_GATEWAYCMD 12
struct SynLevelGatewayCmd: public GatewayCmd
{
  QWORD userid;
  WORD level;
  SynLevelGatewayCmd():GatewayCmd(SYN_LEVEL_GATEWAYCMD)
  {
    userid=0;
    level=0;
  }
};

//消息过滤
#define CMD_FILTER_GATEWAYCMD 13
struct CmdFilterGatewayCmd: public GatewayCmd
{
  BYTE cmd;
  BYTE param;
  BYTE type;	//1屏蔽 2开启
  CmdFilterGatewayCmd():GatewayCmd(CMD_FILTER_GATEWAYCMD)
  {
    cmd = 0;
    param = 0;
    type = 0;
  }
};

//转发消息至其它服务器，（用于暂时存储消息，防止多线程发送消息导致宕机）
#define FORWARD_CMD_TO_SERVER_GATEWAY_CMD 14
struct ForwardCmdToServerGatewayCmd : public GatewayCmd
{
  char serverName[MAX_NAMESIZE];
  bool toFirst;
  WORD len;
  BYTE data[0];
  ForwardCmdToServerGatewayCmd():GatewayCmd(FORWARD_CMD_TO_SERVER_GATEWAY_CMD)
  {
    bzero(serverName, sizeof(serverName));
    toFirst = false;
    len = 0;
  }
};

//网关转发客服用户指令
#define FORWARD_KEFUCMD_GATEWAYCMD 15
struct ForwardKefuCmdGatewayCmd : public GatewayCmd
{
  QWORD userID;
  QWORD accid;
  DWORD zoneID;
  WORD len;
  BYTE data[0];
  ForwardKefuCmdGatewayCmd():GatewayCmd(FORWARD_KEFUCMD_GATEWAYCMD)
  {
    userID = 0;
    accid = 0;
    zoneID = 0;
    len = 0;
  }
};

// 通知网关登录成功
#define GATEUSER_ONLINE_GATEWAYCMD 16
struct GateUserOnlineGatewayCmd : public GatewayCmd
{
  QWORD accid;
  GateUserOnlineGatewayCmd():GatewayCmd(GATEUSER_ONLINE_GATEWAYCMD)
  {
    accid = 0;
  }
};

/*网关转发用户RECORD指令*/
#define FORWARD_RECORDCMD_GATEWAYCMD 17
struct ForwardRecrodCmdGatewayCmd : public GatewayCmd
{
  WORD len;
  BYTE data[0];
  ForwardRecrodCmdGatewayCmd() :GatewayCmd(FORWARD_RECORDCMD_GATEWAYCMD)
  {
    len = 0;
  }
};

/* 同步队伍id */
#define SYN_TEAM_USERCMD_GATEWAYCMD 18
struct SynTeamUserCmdGatewayCmd : public GatewayCmd
{
  QWORD accid = 0;
  QWORD teamid = 0;
  SynTeamUserCmdGatewayCmd() : GatewayCmd(SYN_TEAM_USERCMD_GATEWAYCMD)
  {
  }
};

#pragma pack()
