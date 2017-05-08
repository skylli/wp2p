/*******************************************************************************

    This file is part of the up2p.
    Copyright wilddog.com
    All right reserved.

    File:    up2pc_vc.c

    No description

    TIME LIST:
    CREATE  skyli   2017-05-05 13:48:12

*******************************************************************************/

#include "up2pc.h"
#include "ulink.h"

static u16 local_port, server_port;
static u32 local_ip, server_ip;
static u32 dev0, dev1, key0, key1;

static SOCKET socket_main;
static char recvbuf[1024];

static int recv_proc()
{
	int size;
	struct sockaddr_in sin_recv;
	int addrlen = sizeof(SOCKADDR);

	while(1)
	{
		UDP_INFO info;

		size = recvfrom(socket_main, recvbuf, sizeof(recvbuf), 0, (SOCKADDR*)&sin_recv, &addrlen);
		//printf("recv type:%d idx:%d\n", packet->type, packet->idx);
#ifdef _WIN32
		if(size <= 0)
		{
			printf("WSAGetLastError %d\n", WSAGetLastError());
		}
		// win32 平台上模拟随机"丢包"
        if (rand()&0x01)
        {
            recvbuf[0] = 0x00;
            recvbuf[1] = 0x00;
            recvbuf[2] = 0x00;
            continue;
        }
#endif

		info.sip = sin_recv.sin_addr.s_addr;
		info.sport = sin_recv.sin_port;
		info.dip = 0;	// 实际上用不到这两个值
		info.dport = 0;

		up2pc_recv(recvbuf, size, &info);
	}

	return 0;
}

static u32 dns_ip(const char *sname)
{
	struct hostent *host;

	host = gethostbyname(sname);
	if(host)
	{
		struct in_addr *addr = (struct in_addr *)host->h_addr_list[0];
		server_ip = addr->s_addr;
	}
	else
	{
		server_ip = inet_addr(sname);
	}

	if(server_ip == 0xFFFFFFFF)
	{
		printf("link server error\n");
		server_ip = 0;
	}

	return server_ip;
}

// port
int main(int argc, const char *argv[])
{
	struct sockaddr_in sin;
	
#ifdef _WIN32
	DWORD dwBytesReturned = 0;
	BOOL bNewBehavior = FALSE;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2),&wsaData);//1.1版本
#else
	pthread_t thread;
#endif

	if(argc != 6)
	{
		printf("local_port server_ip server_port dev0 dev1 key0 key1\n");
		return -1;
	}
	
	local_port = atoi(argv[1]);
	//server_ip = inet_addr(argv[2]);
	server_ip = dns_ip(argv[2]);
	server_port = htons(atoi(argv[3]));
	
    ascii_to_2u32(argv[4],&dev0,&dev1);
    ascii_to_2u32(argv[5],&key0,&key1);
	// 创建socket
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(local_port);
	socket_main = socket(AF_INET,SOCK_DGRAM, 0);
#ifdef _WIN32
	WSAIoctl(socket_main, SIO_UDP_CONNRESET,
		&bNewBehavior, sizeof(bNewBehavior),
		NULL, 0, &dwBytesReturned,
		NULL, NULL);
#endif
	bind(socket_main, (SOCKADDR*)&sin, sizeof(SOCKADDR));

	// 设置各种参数
	up2pc_setkey(key0, key1);
	up2pc_set_server(server_ip, server_port);
	up2pc_set_local(0, htons(local_port));
	up2pc_set_name(dev0, dev1);

#ifdef WIN32
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recv_proc, NULL, 0, 0);
#else
	pthread_create(&thread, NULL, (void *)recv_proc, NULL);
#endif
	// 更新服务器信息
	up2pc_update_host();

	while(1)
	{
		// 查询在线状态
		int online = up2pc_get_online();
		
		printf("online = %d\n", online);

		// 定时发送在线信息
		up2pc_set_online();
		
#ifdef _WIN32
		Sleep(5000);
#else
		sleep(5);
#endif
	}

#ifdef _WIN32
	WSACleanup();
#endif

	return 0;
}

// UDP发送数据包
int udp_send(const char *data, int len, UDP_INFO *info)
{
	int addrlen = sizeof(SOCKADDR);
	int ret;
	struct sockaddr_in dstaddr;

	dstaddr.sin_family = AF_INET;
	dstaddr.sin_addr.s_addr = info->dip;
	dstaddr.sin_port = info->dport;

	ret = sendto(socket_main, data, len, 0, (SOCKADDR*)&dstaddr, addrlen);

	return ret;
}

// 获取系统时间
u32 sys_get_time()
{
	return time(NULL);
}

// 保存密钥
int sys_set_key(u32 key0, u32 key1)
{
	printf("update key %x %x\n", key0, key1);
	return 0;
}

// 生成随机数
u32 sys_get_random()
{
	return rand() | (rand() << 16);
}

// 配置服务器信息
int sys_config_host(char *host, u16 port)
{
	u32 server_ip = dns_ip(host);
	server_port = port;

	// 解析成功则,设置服务器信息
	if(server_ip != 0)
	{
		up2pc_set_server(server_ip, server_port);
		// 更新服务器信息
		up2pc_update_host();
	}

	return 0;
}
