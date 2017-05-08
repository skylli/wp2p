/*******************************************************************************

    This file is part of the up2p.
    Copyright wilddog.cn.
    All right reserved.

    File:    up2ps.c

    No description

    TIME LIST:
    CREATE  skyli   2017-05-06 13:48:30

*******************************************************************************/

#include "up2ps.h"

#define MAX_HOST 32

SOCKET socket_main;
static char recvbuf[1024];
static int server_port;

DEVINFO *devinfolist;
int user_total;

#define MAX_UID 0x1000

typedef struct{
	u32 uid0;
	u32 uid1;
	u32 ip;
	u16 port;
} UID_ITEM;

UID_ITEM *uid_list;
static int uid_count;

FILE *debug_log_fp = NULL;

char *time_str()
{
	time_t rawtime;
	struct tm* timeinfo;
	static char time_s[256];
	time(&rawtime);
	timeinfo=localtime(&rawtime);
	strftime(time_s, sizeof(time_s), "%Y-%m-%d %I:%M:%S ",timeinfo);
	return time_s;
}

int debug_log(const char *fmt, ...)
{
	char buffer[0x1000];
	char *time_s = time_str();
	va_list argptr;
	va_start(argptr, fmt);
	vsprintf(buffer,fmt,argptr);
	va_end(argptr);

	if(debug_log_fp == NULL)
		debug_log_fp = fopen("up2ps.log", "w");
	fputs(time_s, debug_log_fp);
	fputs(buffer, debug_log_fp);
	fflush(debug_log_fp);
	printf("%s", time_s);
	printf("%s", buffer);

	return 0;
}

static int user_init()
{
	int i;

	devinfolist = (DEVINFO *)malloc(sizeof(DEVINFO) * MAX_USER);
	memset(devinfolist, 0, sizeof(DEVINFO) * MAX_USER);
	user_total = 0;

#if 0
	for(i = 0; i < MAX_USER; i++)
	{
		devinfolist[i].dev0 = i;
		devinfolist[i].dev1 = i;
		devinfolist[i].ip = i;
		devinfolist[i].port = 0;
		devinfolist[i].time = 0;
	}
#endif
	return 0;
}

DEVINFO *user_find(u32 dev0, u32 dev1)
{
	int i;
	u32 now_time = time(NULL);

	// 寻找空闲块
	for(i = 0; i < MAX_USER; i++)
	{
		// 相同的dev
		if(devinfolist[i].dev0 == dev0 && devinfolist[i].dev1 == dev1)
		{
			return &devinfolist[i];
		}
	}

	return NULL;
}

int user_add(u32 dev0, u32 dev1, u32 ip, u16 port)
{
	int i;
	u32 now_time = time(NULL);

	// 寻找空闲块
	for(i = 0; i < MAX_USER; i++)
	{
		// 超时的或者空闲的,都可以使用
		if((now_time - devinfolist[i].time) > UP2P_ONLINE_TIME 
			|| devinfolist[i].time == 0
			|| (devinfolist[i].dev0 == dev0 && devinfolist[i].dev1 == dev1))
		{
			devinfolist[i].dev0 = dev0;
			devinfolist[i].dev1 = dev1;
			devinfolist[i].ip = ip;
			devinfolist[i].port = port;
			devinfolist[i].time = now_time;
			debug_log("update dev %x %x\n", dev0, dev1);
			break;
		}
	}

	return 0;
}

int user_deinit()
{
	free(devinfolist);

	return 0;
}

// UDP发送数据包
int _send(const char *data, int len, u32 ip, u16 port)
{
	int addrlen = sizeof(SOCKADDR);
	int ret;
	struct sockaddr_in dstaddr;

	dstaddr.sin_family = AF_INET;
	dstaddr.sin_addr.s_addr = ip;
	dstaddr.sin_port = port;

	ret = sendto(socket_main, data, len, 0, (SOCKADDR*)&dstaddr, addrlen);

	return ret;
}

static int recv_cmd(UP2P_PACKET *packet, u32 ip, u16 port)
{
	DEVINFO *devinfo;
	UP2P_HOST *host;
	u32 tmp0, tmp1;

	// 将源端加入列表
	user_add(packet->src0, packet->src1, ip, port);

	debug_log("cmd %d\n", packet->cmd);

	switch(packet->cmd)
	{
	case CMD_ONLINE:
		// 回应ACK
		packet->cmd = CMD_ONLINE_ACK;
		packet->dst0 = packet->src0;
		packet->dst1 = packet->src1;
		packet->src0 = 0;
		packet->src1 = 0;
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

	case CMD_GET_HOST:
		// 获取主机信息
		host = packet->payload;
		host->ip = uid_list[0].ip;
		host->port = uid_list[0].port;
		tmp0 = packet->src0;
		tmp1 = packet->src1;
		packet->src0 = packet->dst0;
		packet->src1 = packet->dst1;
		packet->dst0 = tmp0;
		packet->dst1 = tmp1;
		packet->cmd = CMD_GET_HOST_ACK;
		packet->len = sizeof(UP2P_HOST);
		_send((const char *)packet, sizeof(UP2P_PACKET) + packet->len, ip, port);
		break;

	default:
		// 其他的指令,统一做转发处理
		devinfo = user_find(packet->dst0, packet->dst1);
		if(devinfo)
		{
			debug_log("relay %x %x\n", packet->dst0, packet->dst1);
			_send((const char *)packet, sizeof(UP2P_PACKET) + packet->len, devinfo->ip, devinfo->port);
		}
		else
		{
			debug_log("not found %x %x\n", packet->dst0, packet->dst1);
		}
		break;
	}
	return 0;
}

static int recv_proc(void *param)
{
	int size;
	struct sockaddr_in sin_recv;
	int addrlen = sizeof(SOCKADDR);
	UP2P_PACKET *packet;

	while(1)
	{
		size = recvfrom(socket_main, recvbuf, sizeof(recvbuf), 0, (SOCKADDR*)&sin_recv, &addrlen);

		debug_log("recv from:%s len:%d\n", inet_ntoa(sin_recv.sin_addr), size);

#ifdef _WIN32
		if(size <= 0)
		{
			debug_log("WSAGetLastError %d\n", WSAGetLastError());
		}
#endif

		packet = (UP2P_PACKET *)recvbuf;
		if(packet->magic != MAGIC)
		{
			if(packet->magic == 0x84F5AD90)
			{
				int old_recv_cmd(void *packet, u32 ip, u16 port);
				old_recv_cmd(packet, sin_recv.sin_addr.s_addr, sin_recv.sin_port);
			}
			continue;
		}

		recv_cmd(packet, sin_recv.sin_addr.s_addr, sin_recv.sin_port);
	}

	return 0;
}

int uid_init(const char *fname)
{
	FILE *fp;
	char buf[256];
	int i;

	fp = fopen(fname, "rb");
	if(fp == NULL)
		goto _err;

	uid_list = malloc(MAX_UID * sizeof(UID_ITEM));
	if(uid_list == NULL)
		goto _err2;

	for(i = 0; i < MAX_UID; i++)
	{
		char ipstr[32];
		u32 port;
		char *ret = fgets(buf, sizeof(buf), fp);
		if(ret == NULL)
			break;
		sscanf(ret, "%08x%08x %s %d", &uid_list[i].uid1, &uid_list[i].uid0, ipstr, &port);
		
		uid_list[i].ip = inet_addr(ipstr);
		uid_list[i].port = htons(port);
	}

	uid_count = i;

	fclose(fp);
	
	return 0;
_err2:
	fclose(fp);
_err:
	debug_log("uid_init error\n");
	return -1;
}

int uid_deinit()
{
	free(uid_list);

	return 0;
}

int main(int argc, const char *argv[])
{
	struct sockaddr_in sin;

#ifdef _WIN32
	DWORD dwBytesReturned = 0;
	BOOL bNewBehavior = FALSE;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2),&wsaData);//1.1版本
#endif
	
	debug_log("UP2PS build %s %s\n", __DATE__, __TIME__);

	if(argc !=2)
	{
		debug_log("server_port\n");
		return 0;
	}

	server_port = atoi(argv[1]);

	if(uid_init("up2ps.ini") == -1)
	{
		debug_log("hostlist_init error\n");
		return 0;
	}

	user_init();

	// 创建socket
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(server_port);
	socket_main = socket(AF_INET,SOCK_DGRAM, 0);
#ifdef _WIN32
	WSAIoctl(socket_main, SIO_UDP_CONNRESET,
		&bNewBehavior, sizeof(bNewBehavior),
		NULL, 0, &dwBytesReturned,
		NULL, NULL);
#endif
	bind(socket_main, (SOCKADDR*)&sin, sizeof(SOCKADDR));

	ctrl_init();

	recv_proc(NULL);

	user_deinit();

	uid_deinit();

#ifdef _WIN32
	WSACleanup();
#endif

	return 0;
}
