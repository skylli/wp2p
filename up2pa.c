/*******************************************************************************

    This file is part of the up2p.
    Copyright wilddog.com
    All right reserved.

    File:    up2pa.c

    No description

    TIME LIST:
    CREATE  skyli   2017-05-06 13:46:51

*******************************************************************************/

#include "up2pa.h"
#include "up2p_crypt.h"
#include "flash_map.h"

#ifdef __ANDROID__
#include <android/log.h>
#endif

#define MAX_USER 256

static SOCKET socket_main;;
static u16 local_port;
static u16 server_port;
static u32 server_ip;
static u16 client_port;

static char recvbuf[1024];
static char sendbuf[1024];
static u32 local_mac0;
static u32 local_mac1;

#ifdef _WIN32

void delay_ms(u32 ms)
{
	Sleep(ms);
}
#else
void delay_ms(u32 ms)
{
	usleep(ms * 1000);
}
#endif

int up2pa_debug_level = 0;

static char *time_str()
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
	if(up2pa_debug_level > 0)
	{
		char buffer[0x1000];
		char *time_s = time_str();
		va_list argptr;
		va_start(argptr, fmt);
		vsprintf(buffer,fmt,argptr);
		va_end(argptr);

#ifdef __ANDROID__
		__android_log_print(ANDROID_LOG_INFO, "ulink", "%s", buffer);
#else
		printf("%s", time_s);
		printf("%s", buffer);
#endif
	}
    
	return 0;
}

static volatile int flag_run;
static volatile int flag_break;
static volatile u16 packet_idx;
UP2P_PACKET *packetrecv;
static FILE *fcopy = NULL;

// ��������
typedef enum{
	ONLINE_NO,
	ONLINE_WAN,
	ONLINE_LAN,
	ONLINE_MAX
} ONLINE_TYPE;

typedef struct{
	u32 dev0;
	u32 dev1;
	u32 ip;
	u16 port;
	u16 idx;
	int online;
} ONLINE;

static ONLINE *onlinelist;

// --------------------------------------------------------------------
// ���ݻ��洦����
static int packetrecv_init()
{
	packetrecv = (UP2P_PACKET *)malloc(sizeof(UP2P_PACKET) * MAX_USER);
	memset(packetrecv, 0, sizeof(UP2P_PACKET) * MAX_USER);

	return 0;
}

static UP2P_PACKET *packetrecv_find(u32 dev0, u32 dev1)
{
	int i;
	u32 now_time = (u32)time(NULL);

	// Ѱ�ҿ��п�
	for(i = 0; i < MAX_USER; i++)
	{
		// ��ͬ��dev
		if(packetrecv[i].src0 == dev0 && packetrecv[i].src1 == dev1 
			&& packetrecv[i].magic == MAGIC)
		{
			return &packetrecv[i];
		}
	}

	return NULL;
}

static int packetrecv_add(UP2P_PACKET *packet, int len)
{
	int i;
	u32 now_time = (u32)time(NULL);

	// Ѱ�ҿ��п�
	for(i = 0; i < MAX_USER; i++)
	{
		// ��ʱ�Ļ��߿��е�,������ʹ��
		if(packetrecv[i].magic == 0)
		{
			memcpy(&packetrecv[i], packet, len);
			debug_log("packetrecv_add %x %x\n", packet->src0, packet->src1);
			break;
		}
	}

	return 0;
}

static int packetrecv_remove(UP2P_PACKET *packet)
{
	memset(packet, 0, sizeof(UP2P_PACKET));
	return 0;
}

static int packetrecv_deinit()
{
	free(packetrecv);

	return 0;
}


static int online_init()
{
	onlinelist = (ONLINE *)malloc(sizeof(ONLINE) * MAX_USER);
	memset(onlinelist, 0, sizeof(ONLINE) * MAX_USER);

	return 0;
}

static int online_update(u32 dev0, u32 dev1, u16 idx, int online)
{
	int i;

	// Ѱ�ҿ��п�
	for(i = 0; i < MAX_USER; i++)
	{
		// ��ʱ�Ļ��߿��е�,������ʹ��
		if((onlinelist[i].dev0 == dev0 && onlinelist[i].dev1 == dev1)
			|| (onlinelist[i].dev0 == 0 && onlinelist[i].dev1 == 0))
		{
			onlinelist[i].dev0 = dev0;
			onlinelist[i].dev1 = dev1;

			// һ����LANģʽ,��ʹ�յ���WANģʽ,Ҳһֱ��Ϊ��LANģʽ
			if(onlinelist[i].online != ONLINE_LAN)
			{
				onlinelist[i].online = online;
				onlinelist[i].idx = idx;
			}

			// ����һ��, ˵���Ƕ�ʱ�����ظ���Ӧ�����ݰ�
			if(onlinelist[i].idx == idx)
			{
				// ��ʱ�����ظ������ݰ�, �����LANģʽ, ���滻֮
				if(online == ONLINE_LAN)
				{
					onlinelist[i].online = online;
					onlinelist[i].idx = idx;
					debug_log("online_update %x %x type %d i = %d\n", dev0, dev1, online, i);
				}
			}
			else
			{
				onlinelist[i].online = online;
				onlinelist[i].idx = idx;
				debug_log("online_update %x %x type %d i = %d\n", dev0, dev1, online, i);
			}

			break;
		}
	}
	return 0;
}

static ONLINE *online_find(u32 dev0, u32 dev1)
{
	int i;

	if(dev0 == 0 && dev1 == 0)
		return NULL;

	for(i = 0; i < MAX_USER; i++)
	{
		if(onlinelist[i].dev0 == dev0 && onlinelist[i].dev1 == dev1)
		{
			debug_log("online_find %x %x type %d i = %d\n", dev0, dev1, onlinelist[i].online, i);
			return &onlinelist[i];
		}
	}
	return NULL;
}

static int online_check(u32 dev0, u32 dev1)
{
	ONLINE *online = online_find(dev0, dev1);

	if(online)
	{
		return online->online;
	}

	return -1;
}

static int online_deinit()
{
	free(onlinelist);

	return 0;
}

static int _send(const char *data, int len, u32 ip, u16 port)
{
	int addrlen = sizeof(SOCKADDR);
	int ret;
	struct sockaddr_in dstaddr;

	if(ip == 0)
		return -1;

	dstaddr.sin_family = AF_INET;
	dstaddr.sin_addr.s_addr = ip;
	dstaddr.sin_port = port;

	ret = sendto(socket_main, data, len, 0, (SOCKADDR*)&dstaddr, addrlen);

	return ret;
}

static int send_packet(UP2P_PACKET *packet)
{
	int len;
	ONLINE *online;

	online = online_find(packet->dst0, packet->dst1);
	len = sizeof(UP2P_PACKET) + packet->len;

	debug_log("send dev %x %x idx %x cmd %x\n", packet->dst0, packet->dst1, packet->idx, packet->cmd);

	// ���Ŀ���豸�Ǵ��ڹ��������Ǿ�����
	if(online)
	{
		if(online->online == ONLINE_LAN)
		{
			// �Ǿ�����,ֱ��ʹ�þ�����ģʽ����
			return _send((const char *)packet, len, online->ip, online->port);
		}
	}

	return _send((const char *)packet, len, server_ip, server_port);
}

static int send_packet_to(UP2P_PACKET *packet, u32 ip, u16 port)
{
	int len;

	len = sizeof(UP2P_PACKET) + packet->len;

	return _send((const char *)packet, len, ip, port);
}

// ����������������
static int recv_cmd(UP2P_PACKET *packet, u32 ip, u16 port)
{
	switch(packet->cmd)
	{
	case CMD_CHECK_ONLINE_YES:
		online_update(packet->dst0, packet->dst1, packet->idx, ONLINE_WAN);
		break;
	case CMD_CHECK_ONLINE_NO:
		online_update(packet->dst0, packet->dst1, packet->idx, ONLINE_NO);
		break;
	case CMD_LAN_SCAN_ACK:
		{
			ONLINE *online;
			online_update(packet->src0, packet->src1, packet->idx, ONLINE_LAN);
			online = online_find(packet->src0, packet->src1);
			online->ip = ip;
			online->port = port;
#ifdef _WIN32
			printf("CMD_LAN_SCAN_ACK %x %x\n", packet->src0, packet->src1);
			printf("\n");
#endif
			break;
		}
	default:
		{
			packetrecv_add(packet, sizeof(UP2P_PACKET) + packet->len);
			debug_log("recv cmd = %d\n", packet->cmd);
		}
		break;
	}
	return 0;
}

// ����
static int recv_proc(void *param)
{
	int size;
	struct sockaddr_in sin_recv;
	int addrlen = sizeof(SOCKADDR);
	UP2P_PACKET *packet;

	while(1)
	{
		size = recvfrom(socket_main, recvbuf, sizeof(recvbuf), 0, (SOCKADDR*)&sin_recv, &addrlen);

#ifdef _WIN32
		if(size <= 0)
		{
			debug_log("WSAGetLastError %d\n", WSAGetLastError());
		}
#endif
		packet = (UP2P_PACKET *)recvbuf;

		debug_log("recv from:%s len:%d magic:%x cmd:%x\n", inet_ntoa(sin_recv.sin_addr), size, packet->magic, packet->cmd);
		
		if(packet->magic != MAGIC)
			continue;
		
		recv_cmd(packet, sin_recv.sin_addr.s_addr, sin_recv.sin_port);
	}

	return 0;
}

/*
 * ��ʼ��
 * sport ���ذ󶨶˿� sname ��������������IP dport �������˿� cport�ͻ��˵Ķ˿�
 */
int up2pa_init(u16 sport, const char *sname, u16 dport, u16 cport, u32 mac0, u32 mac1)
{
	struct hostent *host;
	struct sockaddr_in sin;
	int op0;

#ifdef _WIN32
	DWORD dwBytesReturned = 0;
	BOOL bNewBehavior = FALSE;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2),&wsaData);//1.1�汾
#else
	pthread_t thread;
#endif

	flag_run = 0;
	flag_break = 0;

	local_port = sport;
	server_port = dport;
	client_port = cport;
	local_mac0 = mac0;
	local_mac1 = mac1;
	packet_idx = 0;

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
		debug_log("link server error\n");
		server_ip = 0;
	}

	packetrecv_init();
	online_init();

	// ����socket
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(local_port);
	socket_main = socket(AF_INET, SOCK_DGRAM, 0);
	// ʹ�ܹ㲥
	op0 = 1;
	setsockopt(socket_main, SOL_SOCKET, SO_BROADCAST, (char *)&op0, sizeof(op0));

#ifdef _WIN32
	WSAIoctl(socket_main, SIO_UDP_CONNRESET,
		&bNewBehavior, sizeof(bNewBehavior),
		NULL, 0, &dwBytesReturned,
		NULL, NULL);
#endif
	bind(socket_main, (SOCKADDR*)&sin, sizeof(SOCKADDR));
	
#ifdef WIN32
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recv_proc, NULL, 0, 0);
#else
	pthread_create(&thread, NULL, (void *)recv_proc, NULL);
#endif

	return 0;
}

/*
 * ж��
 */
int up2pa_deinit()
{
	packetrecv_deinit();
	online_deinit();

	closesocket(socket_main);

#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}

#ifdef _WIN32
static int get_bcast_list(u32 *list, int maxlen)
{
	INTERFACE_INFO bInterfaceInfo[10];
	DWORD dlByteReturn = 0;
	int iNumOfInterface;
	int i;

	WSAIoctl(socket_main, SIO_GET_INTERFACE_LIST, NULL, 0, bInterfaceInfo, 1024, &dlByteReturn, NULL, NULL );
	iNumOfInterface = dlByteReturn / sizeof(INTERFACE_INFO);
	for(i = 0; i < iNumOfInterface; i++)
	{
		struct sockaddr_in pIPAddress;
		struct sockaddr_in pMaskAddress;

		if(i >= maxlen)
			break;

		pIPAddress = bInterfaceInfo[i].iiAddress.AddressIn;
		pMaskAddress = bInterfaceInfo[i].iiNetmask.AddressIn;
		list[i] = pIPAddress.sin_addr.s_addr | (~pMaskAddress.sin_addr.s_addr);
	}

	return i;
}
#elif defined(__ANDROID__)
static int get_bcast_list(u32 *list, int maxlen)
{
	int i, num;
	struct ifconf ifc;
	struct ifreq buf[10];
	
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	
	if(ioctl(socket_main, SIOCGIFCONF, (char *)&ifc) != 0)
	{
		debug_log("get_bcast_list err\n");
		return 0;
	}
	
	num = ifc.ifc_len / sizeof(struct ifreq);
	if(num > 10)
		num = 10;
	
	debug_log("num = %d\n", num);
	
	for(i = 0; i < num; i++)
	{
		u32 ip = 0, mask = 0;
		
		if(!(ioctl(socket_main, SIOCGIFADDR, (char *)&buf[i])))
		{
			ip = ((struct sockaddr_in*) (&buf[i].ifr_addr))->sin_addr.s_addr;
			
			debug_log("ip %x\n", ip);
		}
		
		if(!(ioctl(socket_main, SIOCGIFNETMASK, (char *)&buf[i])))
		{
			mask = ((struct sockaddr_in*) (&buf[i].ifr_netmask))->sin_addr.s_addr;
			
			debug_log("mask %x\n", mask);
		}
		
		list[i] = ip | (~mask);
	}
	
	return i;
}
#else
static int get_bcast_list(u32 *list, int maxlen)
{
	int num;
	struct ifaddrs *ifa, *oifa;
    
    if(getifaddrs(&ifa) < 0)
    {
        debug_log("get_bcast_list err\n");
		return 0;
    }
    oifa = ifa;
    num = 0;
    while(ifa)
    {
        struct sockaddr_in* saddr = (struct sockaddr_in*)ifa->ifa_addr;
        struct sockaddr_in* smask = (struct sockaddr_in*)ifa->ifa_netmask;
        
        if(saddr && smask)
        {
            u32 ip = saddr->sin_addr.s_addr;
            u32 mask = smask->sin_addr.s_addr;
            
            if(ip != 0 && ip != 0x100007f)
            {
                list[num] = ip | (~mask);
                //debug_log("bcast %x\n", list[num]);
                num++;
            }
        }
        
        ifa = ifa->ifa_next;
    }
	
	
	return num;
}
#endif

/*
 * ��������״̬
 */
int up2pa_clear_online(u32 dev0, u32 dev1)
{
	ONLINE *online = online_find(dev0, dev1);

	if(online)
	{
		online->dev0 = 0;
		online->dev1 = 0;
		online->online = 0;
	}

	return 0;
}

/*
 * ���ͼ������״̬�����ݰ�, ��ѯ����״̬֮ǰ�ȵ���
 */
int up2pa_send_online(u32 dev0, u32 dev1)
{
	UP2P_PACKET *packet = (UP2P_PACKET *)sendbuf;
	u32 iplist[4];
	int num;
	int i;

	// ���͸�������
	packet->magic = MAGIC;
	packet->src0 = local_mac0;
	packet->src1 = local_mac1;
	packet->dst0 = dev0;
	packet->dst1 = dev1;
	packet->cmd = CMD_CHECK_ONLINE;
	packet->len = 0;

	// online����idxʹ����ʱ���16λ
	packet->idx = (u16)time(NULL);
	
	debug_log("send online to %x %x\n", server_ip, server_port);
	send_packet_to(packet, server_ip, server_port);

	packet->cmd = CMD_LAN_SCAN;

	// ��ȡ���������б�,ÿ��������һ�ι㲥
	num = get_bcast_list(iplist, 4);
	for(i = 0; i < num; i++)
	{
		// ���ڶ˿ں�: �˴��Ķ˿��ǿͻ��˵Ķ˿�
		send_packet_to(packet, iplist[i], client_port);
	}
	return 0;
}

/*
 * �������״̬
 * ����ֵ 0 ������ 1 ���������� 2 ���������� -1 ����������Ӧ
 */
int up2pa_check_online(u32 dev0, u32 dev1)
{
	return online_check(dev0, dev1);
}

/*
 * ����Զ��ָ��
 * cmd ָ����� param ָ������� len �������ݳ���
 * û�и������� param ����ΪNULL, lenΪ0
 */
int up2pa_send_cmd(u32 dev0, u32 dev1, u32 cmd, const char *param, int len)
{
	UP2P_PACKET *packet = (UP2P_PACKET *)sendbuf;

	if(flag_break)
		return -2;

	packet->magic = MAGIC;
	packet->src0 = local_mac0;
	packet->src1 = local_mac1;
	packet->dst0 = dev0;
	packet->dst1 = dev1;
	packet->cmd = cmd;
	packet->len = len;

	packet_idx++;
	packet->idx = packet_idx;
	if(len > 0)
		memcpy(packet->payload, param, len);

	return send_packet(packet);
}

/*
 * ���ʹ�������Զ��ָ��
 * idx ������ cmd ָ����� param ָ������� len �������ݳ���
 * û�и������� param ����ΪNULL, lenΪ0
 */
int up2pa_send_idx_cmd(u32 dev0, u32 dev1, u16 idx, u32 cmd, const char *param, int len)
{
	UP2P_PACKET *packet = (UP2P_PACKET *)sendbuf;

	if(flag_break)
		return -2;

	packet->magic = MAGIC;
	packet->src0 = local_mac0;
	packet->src1 = local_mac1;
	packet->dst0 = dev0;
	packet->dst1 = dev1;
	packet->cmd = cmd;
	packet->len = len;
	packet->idx = idx;
	if(len > 0)
		memcpy(packet->payload, param, len);

	return send_packet(packet);
}

/*
 * ���ͼ���Զ��ָ��
 * cmd ָ����� param ָ������� len �������ݳ���
 * û�и������� param ����ΪNULL, lenΪ0
 */
int up2pa_send_data_cmd(u32 dev0, u32 dev1, u32 key0, u32 key1, u32 token, u32 cmd, const char *param, int len)
{
	int ret;
	char inbuf[512];
	char outbuf[512];
	UP2P_DATA *data = (UP2P_DATA *)inbuf;

	data->cmd = cmd;
	data->token = token;
	data->len = len;
	
	if(param)
	{
		memcpy(data->payload, param, len);
	}
	
	debug_log("send_data_cmd token = %x cmd = %x\n", token, cmd);

	len = data_enc((char *)data, outbuf, sizeof(UP2P_DATA) + data->len, key0, key1);
	ret = up2pa_send_cmd(dev0, dev1, CMD_DATA, outbuf, len);

	return ret;
}

/*
 * ��ȡ����ָ��
 * data ���ջ��� maxlen ��������󳤶�
 * ����ֵ �������ݵĳ��� ����Ҳ����Ϊ0 -1 ������
 */
int up2pa_recv_cmd(u32 dev0, u32 dev1, u32 *cmd, char *data, int maxlen)
{
	int len;

	UP2P_PACKET *packet = packetrecv_find(dev0, dev1);

	if(flag_break)
		return -2;

	if(packet == NULL)
		return -1;

	if(packet->len > maxlen || packet->idx != packet_idx)
	{
		packetrecv_remove(packet);
		return -1;
	}

	*cmd = packet->cmd;
	len = packet->len;

	if(len > 0)
	{
		memcpy(data, packet->payload, packet->len);
	}

	packetrecv_remove(packet);

	return len;
}

/*
 * ��ȡ�������ķ���ָ��
 * idx ������ data ���ջ��� maxlen ��������󳤶�
 * ����ֵ �������ݵĳ��� ����Ҳ����Ϊ0 -1 ������
 */
int up2pa_recv_idx_cmd(u32 dev0, u32 dev1, u16 idx, u32 *cmd, char *data, int maxlen)
{
	int len;

	UP2P_PACKET *packet = packetrecv_find(dev0, dev1);

	if(flag_break)
		return -2;

	if(packet == NULL)
		return -1;

	if(packet->len > maxlen || packet->idx != idx)
	{
		packetrecv_remove(packet);
		return -1;
	}

	*cmd = packet->cmd;
	len = packet->len;

	if(len > 0)
	{
		memcpy(data, packet->payload, packet->len);
	}

	packetrecv_remove(packet);

	return len;
}

/*
 * ��ȡ���ؼ���ָ��
 * data ���ջ��� maxlen ��������󳤶�
 * ����ֵ �������ݵĳ��� ����Ҳ����Ϊ0 -1 ������
 */
int up2pa_recv_data_cmd(u32 dev0, u32 dev1, u32 key0, u32 key1, u32 token, u32 *cmd, char *data, int maxlen)
{
	int len;

	char inbuf[512];
	char outbuf[512];
	u32 rcmd;
	UP2P_DATA *udata = (UP2P_DATA *)outbuf;

	// ��ȡһ�����ݰ�
	len = up2pa_recv_cmd(dev0, dev1, &rcmd, inbuf, sizeof(inbuf));
	if(len < 0)
		return len;
	if(rcmd != CMD_DATA_ACK)
		return -1;
	if(len > 512)
		return -1;

	// �������ݰ�
	len = data_dec(inbuf, (char *)udata, len, key0, key1);
	if(udata->token != token)
	{
		debug_log("diff token recv %x my %x\n", udata->token, token);
		return -1;
	}
	debug_log("ok token recv %x my %x\n", udata->token, token);
	if(udata->len > maxlen)
	{
		debug_log("udata->len > maxlen\n");
		return -1;
	}

	// �������
	*cmd = udata->cmd;
	if(data)
		memcpy(data, udata->payload, udata->len);
	
	return udata->len;
}

static int common_wait_cmd(u32 dev0, u32 dev1, u32 key0, u32 key1, u32 token, u32 cmd, char *data, int maxlen, int crypt)
{
	int len;
	int retry;
	u32 recvcmd;
	int time_start = (int)time(NULL);

	retry = 0;
	while(1)
	{
		if(flag_break)
			return -2;

		if(crypt)
		{
			len = up2pa_recv_data_cmd(dev0, dev1, key0, key1, token, &recvcmd, data, maxlen);
		}
		else
		{	
			len = up2pa_recv_cmd(dev0, dev1, &recvcmd, data, maxlen);
		}

		if(len >= 0)
		{
			if(recvcmd == cmd)
				break;
			debug_log("same idx but diff cmd recv %x my %x, don't retry\n", recvcmd, cmd);
			len = -1;
			break;
		}

		if(time(NULL) - time_start > MAX_WAIT_TIME)
			break;
		
		// ����500ms֮��,ÿ��500ms�ط����һ�����ݰ�
		if(retry >= 50 && (retry % 50) == 0)
		{
			UP2P_PACKET *packet = (UP2P_PACKET *)sendbuf;
			send_packet(packet);
		}

		delay_ms(10);
		retry++;
	}
	return len;
}

/*
 * �������ȴ�ָ����ָ��
 * cmd �ȴ���ָ�� data ���ջ��� maxlen ��������󳤶�
 * ����ֵ �������ݵĳ��� ����Ҳ����Ϊ0 -1 ������
 */
int up2pa_wait_cmd(u32 dev0, u32 dev1, u32 cmd, char *data, int maxlen)
{
	return common_wait_cmd(dev0, dev1, 0, 0, 0, cmd, data, maxlen, 0);
}

/*
 * �������ȴ�ָ���ļ���ָ��
 * cmd �ȴ���ָ�� data ���ջ��� maxlen ��������󳤶�
 * ����ֵ �������ݵĳ��� ����Ҳ����Ϊ0 -1 ������
 */
int up2pa_wait_data_cmd(u32 dev0, u32 dev1, u32 key0, u32 key1, u32 token, u32 cmd, char *data, int maxlen)
{
	return common_wait_cmd(dev0, dev1, key0, key1, token, cmd, data, maxlen, 1);
}

/*
 * ���÷���������
 */
int up2pa_set_host(u32 sip, u16 sport)
{
	server_ip = sip;
	server_port = sport;

	return 0;
}

/*
 * �߽籣����ʼ
 * �ڶ��߳���ͨѶ����ʹ�ô˺�����Ϊ��ʼ
 */
void up2pa_border_begin()
{
	flag_break = 1;
	while(flag_run)
	{
		delay_ms(10);
	}
	flag_run = 1;
	flag_break = 0;
}

/*
 * �߽籣������
 * �ڶ��߳���ͨѶ����ʹ�ô˺�����Ϊ����
 */
void up2pa_border_end()
{
	flag_run = 0;
}


