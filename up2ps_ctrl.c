/*******************************************************************************

    This file is part of the up2p.
    Copyright wilddog.com
    All right reserved.

    File:    up2ps_ctrl.c

    No description

    TIME LIST:
    CREATE  skyli   2017-05-06 10:58:11

*******************************************************************************/

#include "up2ps.h"
#include "up2ps_ctrl.h"

//sprintf(buf, "%08X%08X", dev0, dev1);

static void _proc_list(SOCKET socket_client, CTRL_LOGIN *head)
{
	// �ط����е��û��б�
	int i, j, cnt;
	CLIENT_INFO_HEAD info_head;
	CLIENT_INFO *info;

	cnt = 0;
	for(i = 0; i < MAX_USER; i++)
	{
		if(devinfolist[i].dev0 != 0 || devinfolist[i].dev1 != 0)
		{
			cnt++;
		}
	}

	// �����û�����
	info_head.magic = CTRL_MAGIC;
	info_head.count = cnt;

	send(socket_client, &info_head, sizeof(CLIENT_INFO_HEAD), 0);

	// ѭ�������û�
	j = 0;
	info = malloc(sizeof(CLIENT_INFO) * cnt);
	for(i = 0; i < MAX_USER; i++)
	{
		if(devinfolist[i].dev0 != 0 || devinfolist[i].dev1 != 0)
		{
			info[j].dev0 = devinfolist[i].dev0;
			info[j].dev1 = devinfolist[i].dev1;
			info[j].ip = devinfolist[i].ip;
			info[j].key0 = 0;
			info[j].key1 = 0;
			info[j].time = devinfolist[i].time;

			j++;
			if(j >= cnt)
				break;
		}
	}

	send(socket_client, info, sizeof(CLIENT_INFO) * cnt, 0);

	free(info);
}

static int _client_proc(SOCKET socket_client)
{
	CTRL_LOGIN head;
	int len;

	debug_log("client connect\n");

	len = recv(socket_client, (char *)&head, sizeof(head), 0);
	if(len < 0 || len != sizeof(head))
	{
		debug_log("recv client head error\n");
		goto _err;
	}

	if(head.magic != CTRL_MAGIC)
	{
		debug_log("client magic error\n");
		goto _err;
	}
	
	switch(head.cmd)
	{
	case CTRL_CMD_LIST:
		_proc_list(socket_client, &head);
		break;
	}
	
_err:
	shutdown(socket_client, SD_BOTH);
	closesocket(socket_client);
	debug_log("client disconnect\n");
	return 0;
}

int ctrl_proc(void *param)
{
	SOCKET socket_main;
	struct sockaddr_in addr_main, addr_client;
	int ret;

#ifdef _WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

	socket_main = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	addr_main.sin_family = AF_INET;
    addr_main.sin_port = htons(SERVER_PORT); 
    addr_main.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	ret = bind(socket_main, (struct sockaddr *)&addr_main, sizeof(addr_main));
	ret = listen(socket_main, MAX_USERS);

	//memset(user_list, 0, sizeof(user_list));

	while(1)
	{
		SOCKET socket_client;
		int len = sizeof(addr_client);  
        socket_client = accept(socket_main, (struct sockaddr *)&addr_client, &len);

		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)_client_proc, (LPVOID)socket_client, 0, 0);
	}

	closesocket(socket_main);
	return 0;
}

int ctrl_init(void)
{
#ifdef WIN32
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ctrl_proc, NULL, 0, 0);
#else
	pthread_create(&thread, NULL, (void *)ctrl_proc, NULL);
#endif
	return 0;
}
