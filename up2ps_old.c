/*******************************************************************************

    This file is part of the project.
    Copyright wilddogf.com
    All right reserved.

    File:    up2ps_old.c


    TIME LIST:
    CREATE  skyli   2017-0506-11 20:27:22

*******************************************************************************/

#include <stdio.h>
#include <time.h>

//---------------------------------wilddog旧版定义-------------------------------------
// 基本类型
typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef long s32;
typedef unsigned long u32;

// 指令类型  目前不在使用
enum{
	CMD_NONE = 0,			// 空类型
	CMD_ONLINE,				// 在线指令
	CMD_ONLINE_ACK,			// 在线回应
	CMD_CHECK_ONLINE,		// 检查设备是否在线
	CMD_CHECK_ONLINE_YES,	// 确认在线
	CMD_CHECK_ONLINE_NO,	// 确认不在线
	CMD_PING,				// 发送测试指令
	CMD_PING_ACK,			// 测试回应
	CMD_SEND_SERIAL,		// 发送串口数据
	CMD_SEND_SERIAL_ACK,	// 发送串口数据回应
	CMD_READ_SERIAL,		// 读取串口数据
	CMD_READ_SERIAL_ACK,	// 读取串口数据回应
	CMD_LAN_SCAN,			// 扫描局域网设备
	CMD_LAN_SCAN_ACK,		// 回应扫描
	CMD_SEND_KEY,			// 发送新的KEY到客户端
	CMD_SEND_KEY_OK,		// 客户端已经接受新的KEY
	CMD_TXRX_SERIAL,		// 以收发一体的形式发送串口数据  
	CMD_TXRX_SERIAL_ACK,	// 返收发一体的形式回应串口数据
	CMD_MAX
};

// UDP相关参数
typedef struct{
	u32 sip;		// 源IP
	u16 sport;		// 源端口
	u32 dip;		// 目标IP
	u16 dport;		// 目标端口
} UDP_INFO;

// 总包
typedef struct{
	u32 magic;			// 验证数
	u32 dst0;			// 设备名0
	u32 dst1;			// 设备名1
	u32 key0;			// 密钥0
	u32 key1;			// 密钥1
	u32 src0;			// 源设备名0
	u32 src1;			// 源设备名1
	u32 cmd;			// 命令类型
	u16 idx;			// 序号
	u16 len;			// 数据长度
	char payload[0];	// 数据
} UP2P_PACKET;

// 在线时间(秒)
#define UP2P_ONLINE_TIME 60

//-----------------------------必须同步最新定义---------------------------------

typedef struct{
	u32 dev0;		// 设备名
	u32 dev1;
	u32 ip;		// 设备IP
	u16 port;		// 设备端口
	u32 time;
} DEVINFO;

extern DEVINFO *devinfolist;

int user_add(u32 dev0, u32 dev1, u32 ip, u16 port);

int debug_log(const char *fmt, ...);
int _send(const char *data, int len, u32 ip, u16 port);
DEVINFO *user_find(u32 dev0, u32 dev1);

int old_recv_cmd(UP2P_PACKET *packet, u32 ip, u16 port)
{
	DEVINFO *devinfo;

	// 将源端加入列表
	user_add(packet->src0, packet->src1, ip, port);

	debug_log("old cmd %d\n", packet->cmd);

	switch(packet->cmd)
	{
	case CMD_ONLINE:
		// 回应ACK
		packet->cmd = CMD_ONLINE_ACK;
		_send((const char *)packet, sizeof(UP2P_PACKET), ip, port);
		break;

	case CMD_CHECK_ONLINE:
		// 查询在线
		devinfo = user_find(packet->dst0, packet->dst1);
		if(devinfo)
		{
			if(time(NULL) - devinfo->time < UP2P_ONLINE_TIME)
			{
				packet->cmd = CMD_CHECK_ONLINE_YES;
			}
			else
			{
				packet->cmd = CMD_CHECK_ONLINE_NO;
			}
		}
		else
		{
			packet->cmd = CMD_CHECK_ONLINE_NO;
		}
		_send((const char *)packet, sizeof(UP2P_PACKET), ip, port);
		break;

	default:
		// 其他的指令,统一做转发处理
		devinfo = user_find(packet->dst0, packet->dst1);
		if(devinfo)
		{
			debug_log("old relay %x %x\n", packet->dst0, packet->dst1);
			_send((const char *)packet, sizeof(UP2P_PACKET) + packet->len, devinfo->ip, devinfo->port);
		}
		else
		{
			debug_log("old not found %x %x\n", packet->dst0, packet->dst1);
		}
		break;
	}
	return 0;
}
