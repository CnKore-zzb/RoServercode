#pragma once
/*#include "xCommand.h"
#pragma pack(1)

struct SuperCmd : public xCommand
{
	SuperCmd(unsigned char p=0):xCommand(SUPER_CMD, p)
	{
	}
};

#define GATE_LISTEN_SUPERCMD 1
struct GateListenSuperCmd : public SuperCmd
{
	GateListenSuperCmd():SuperCmd(GATE_LISTEN_SUPERCMD)
	{
	}
};

#define GATE_READY_SUPERCMD 2
struct GateReadySuperCmd : public SuperCmd
{
	GateReadySuperCmd():SuperCmd(GATE_READY_SUPERCMD)
	{
	}
};

//更新网关在线人数
#define UPDATE_GATE_USER_NUM_SUPERCMD 3
struct UpdateGateUserNumSuperCmd : public SuperCmd
{
	DWORD ip;
	WORD port;	
	WORD num;
	UpdateGateUserNumSuperCmd():SuperCmd(UPDATE_GATE_USER_NUM_SUPERCMD)
	{
		ip = 0;
		port = 0;
		num = 0;
	}
};

//通知各服务器启动成功
#define SERVER_OK_SUPERCMD 4
struct ServerOkSuperCmd : public SuperCmd
{
	ServerOkSuperCmd():SuperCmd(SERVER_OK_SUPERCMD)
	{
	}
};

//重新加载版本信息
#define RELOAD_VERION_SUPERCMD 5
struct ReloadVersionSuperCmd : public SuperCmd
{
	ReloadVersionSuperCmd() : SuperCmd(RELOAD_VERION_SUPERCMD)
	{
	}
};

****************************************************************/
#pragma pack()
