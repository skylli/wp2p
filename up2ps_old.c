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

//---------------------------------wilddog�ɰ涨��-------------------------------------
// ��������
typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef long s32;
typedef unsigned long u32;

// ָ������  Ŀǰ����ʹ��
enum{
	CMD_NONE = 0,			// ������
	CMD_ONLINE,				// ����ָ��
	CMD_ONLINE_ACK,			// ���߻�Ӧ
	CMD_CHECK_ONLINE,		// ����豸�Ƿ�����
	CMD_CHECK_ONLINE_YES,	// ȷ������
	CMD_CHECK_ONLINE_NO,	// ȷ�ϲ�����
	CMD_PING,				// ���Ͳ���ָ��
	CMD_PING_ACK,			// ���Ի�Ӧ
	CMD_SEND_SERIAL,		// ���ʹ�������
	CMD_SEND_SERIAL_ACK,	// ���ʹ������ݻ�Ӧ
	CMD_READ_SERIAL,		// ��ȡ��������
	CMD_READ_SERIAL_ACK,	// ��ȡ�������ݻ�Ӧ
	CMD_LAN_SCAN,			// ɨ��������豸
	CMD_LAN_SCAN_ACK,		// ��Ӧɨ��
	CMD_SEND_KEY,			// �����µ�KEY���ͻ���
	CMD_SEND_KEY_OK,		// �ͻ����Ѿ������µ�KEY
	CMD_TXRX_SERIAL,		// ���շ�һ�����ʽ���ʹ�������  
	CMD_TXRX_SERIAL_ACK,	// ���շ�һ�����ʽ��Ӧ��������
	CMD_MAX
};

// UDP��ز���
typedef struct{
	u32 sip;		// ԴIP
	u16 sport;		// Դ�˿�
	u32 dip;		// Ŀ��IP
	u16 dport;		// Ŀ��˿�
} UDP_INFO;

// �ܰ�
typedef struct{
	u32 magic;			// ��֤��
	u32 dst0;			// �豸��0
	u32 dst1;			// �豸��1
	u32 key0;			// ��Կ0
	u32 key1;			// ��Կ1
	u32 src0;			// Դ�豸��0
	u32 src1;			// Դ�豸��1
	u32 cmd;			// ��������
	u16 idx;			// ���
	u16 len;			// ���ݳ���
	char payload[0];	// ����
} UP2P_PACKET;

// ����ʱ��(��)
#define UP2P_ONLINE_TIME 60

//-----------------------------����ͬ�����¶���---------------------------------

typedef struct{
	u32 dev0;		// �豸��
	u32 dev1;
	u32 ip;		// �豸IP
	u16 port;		// �豸�˿�
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

	// ��Դ�˼����б�
	user_add(packet->src0, packet->src1, ip, port);

	debug_log("old cmd %d\n", packet->cmd);

	switch(packet->cmd)
	{
	case CMD_ONLINE:
		// ��ӦACK
		packet->cmd = CMD_ONLINE_ACK;
		_send((const char *)packet, sizeof(UP2P_PACKET), ip, port);
		break;

	case CMD_CHECK_ONLINE:
		// ��ѯ����
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
		// ������ָ��,ͳһ��ת������
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
