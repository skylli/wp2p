/*******************************************************************************

    This file is part of the ulink.
    Copyright wilddog.com
    All right reserved.

    File:    ulink.c

    No description

    TIME LIST:
    CREATE  skyli   2014-08-27 13:16:17

*******************************************************************************/

#include "up2pa.h"
#include "ulink.h"

struct ULINK{
	u32 dev0;
	u32 dev1;
	u32 key0;
	u32 key1;
	u32 token;
	u32 sip;
	u16 sport;
};

static char server_name[64];

int StartSmartConnection(const char *, const char *, char);
int StopSmartConnection();


static int ascii_to_2u32(const char *devid, u32 *dev0, u32 *dev1)
{
	char tmp0[32];
	char tmp1[32];
	int i;

	if(strlen(devid) != 16)
		return -1;

	memset(tmp0, 0, sizeof(tmp0));
	memset(tmp1, 0, sizeof(tmp1));

	strncpy(tmp0, devid, 8);
	strncpy(tmp1, devid + 8, 8);

	sscanf(tmp0, "%08x", dev0);
	sscanf(tmp1, "%08x", dev1);

	return 0;
}

/*
 * 初始化库
 * host 服务器域名或者IP地址
 * appid 代表本机唯一的ID码 8字节HEX字符串
 * 返回0成功
 */
int ulink_init(const char *host, const char *appid)
{
	int ret;
	u32 mac0, mac1;
	u16 aport, sport, cport; 
	ascii_to_2u32(appid, &mac0, &mac1);

	debug_log("libulink Build %s %s\n", __DATE__, __TIME__);
	
	strcpy(server_name, host);
	aport = htons(UP2PA_PORT);
	sport = htons(UP2PS_PORT);
	cport = htons(UP2PC_PORT);
	ret = up2pa_init(aport, host, sport, cport, mac0, mac1);

	return 0;
}

/*
 * 卸载库
 */
int ulink_deinit()
{
	up2pa_deinit();

	return 0;
}

static int gen_key(u32 *key0, u32 *key1)
{
	// 生成key
#ifdef _WIN32
	*key0 = GetTickCount() * time(NULL);
	delay_ms(500);
	*key1 = GetTickCount() * time(NULL);
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);

	*key0 = (tv.tv_sec * tv.tv_usec) ^ tv.tv_usec;
	delay_ms(500);
	*key1 = (tv.tv_sec * tv.tv_usec) ^ tv.tv_usec;
#endif

	return 0;
}

/*
 * 配置设备
 * devid 16字节的设备ID字符串
 * ssid 将要连接到的接入点SSID
 * key 接入点的密钥
 * type 加密类型
 * 返回值 ULINK_ERR 当且仅当返回ULINK_ERR_NONE时表示成功
 * outkey 返回密钥字符串
 */
int ulink_config(const char *devid, const char *ssid, const char *key, int type, char *outkey)
{
	u32 token, dev0, dev1, key0, key1;
	int ret, i, status;
	UP2P_TOKEN stk;
	UP2P_HOST host;
    UP2P_CONFIG_HOST cfgh;

	// 配置wifi
#if TARGET_OS_IPHONE
	InitSmartConnection();
#endif
#if defined(__ANDROID__) || defined(TARGET_OS_IPHONE)
	ret = StartSmartConnection(ssid, key, (char)type);
	if(ret != 0)
	{
		debug_log("StartSmartConnection error.\n");
	}
#endif

	// 配置key
	ret = ascii_to_2u32(devid, &dev0, &dev1);
	if(ret == -1)
	{
		ret = ULINK_ERR_DEVID_INVILD;
		goto _err;
	}
	
	debug_log("devid %s dev %x %x\n", devid, dev0, dev1);

	gen_key(&key0, &key1);


	status = ULINK_ERR_DEV_OFFLINE;
	
#if 0
	// 获取服务器信息
	up2pa_send_cmd(dev0, dev1, CMD_GET_HOST, NULL, 0);
	ret = up2pa_wait_cmd(dev0, dev1, CMD_GET_HOST_ACK, &host, sizeof(UP2P_HOST));
	if(ret >= 0)
	{
		// 有服务器信息则保存
		up2pa_set_host(host.ip, host.port);
	}
#endif
	
	// 初始化,设置客户端密钥
	for(i = 0; i < MAX_CONFIG_TIME; i++)
	{
		// 检查在线状态
		up2pa_send_online(dev0, dev1);
		delay_ms(500);

		debug_log("check %x %x\n", dev0, dev1);
		ret = up2pa_check_online(dev0, dev1);
		delay_ms(500);
		debug_log("online = %d\n", ret);

		if(ret == -1)
		{
			status = ULINK_ERR_SERVER_OFFLINE;
			continue;
		}
		else
		if(ret == 0)
		{
			status = ULINK_ERR_DEV_OFFLINE;
			continue;
		}

		// 获取token
		// 令牌获取成功才能进行加密通讯
		up2pa_send_data_cmd(dev0, dev1, 0, 0, 0, CMD_UPDATE_TOKEN, NULL, 0);
		ret = up2pa_wait_data_cmd(dev0, dev1, 0, 0, 0, CMD_UPDATE_TOKEN_ACK, &stk, sizeof(stk));
		debug_log("CMD_UPDATE_TOKEN_ACK ret = %d token = %x\n", ret, stk.token);
		token = stk.token;
		
		if(ret == -1)
		{
			status = ULINK_ERR_INIT_TOKEN;
			continue;
		}

		break;
	}
	if(i >= 20)
	{
		goto _err;
	}

#if defined(__ANDROID__) || defined(TARGET_OS_IPHONE)
	ret = StopSmartConnection();
	if (ret != 0)
	{
		debug_log("StopSmartConnection error.\n");
	}
#endif

	status = ULINK_ERR_CONFIG_HOST;
	for(i = 0; i < 5; i++)
	{
		// 设置服务器地址
		strcpy(cfgh.name, server_name);
		cfgh.port = htons(UP2PS_PORT);

		up2pa_send_data_cmd(dev0, dev1, 0, 0, token, CMD_CONFIG_HOST, &cfgh, sizeof(UP2P_CONFIG_HOST));
		// 等待ACK
		ret = up2pa_wait_data_cmd(dev0, dev1, 0, 0, token, CMD_CONFIG_HOST_ACK, NULL, 0);
		debug_log("CMD_CONFIG_HOST_ACK ret = %d\n", ret);
		if(ret == 0)
		{
			break;
		}
	}
	if(i >= 5)
	{
		goto _err;
	}

	status = ULINK_ERR_INIT_KEY;
	for(i = 0; i < 5; i++)
	{
		UP2P_SET_KEY key;
		u32 recv_cmd;

		key.key0 = key0;
		key.key1 = key1;
		// 发送key
		up2pa_send_data_cmd(dev0, dev1, 0, 0, token, CMD_SEND_KEY, (const char *)&key, sizeof(UP2P_SET_KEY));
		// 等待ACK
		ret = up2pa_wait_data_cmd(dev0, dev1, key0, key1, token, CMD_SEND_KEY_OK, NULL, 0);

		debug_log("CMD_SEND_KEY_OK ret = %d\n", ret);

		if(ret >= 0)
		{
			status = ULINK_ERR_NONE;
			break;
		}

		// 发送key
		up2pa_send_data_cmd(dev0, dev1, key0, key1, token, CMD_SEND_KEY, (const char *)&key, sizeof(UP2P_SET_KEY));
		// 等待ACK
		ret = up2pa_wait_data_cmd(dev0, dev1, key0, key1, token, CMD_SEND_KEY_OK, NULL, 0);
		debug_log("CMD_SEND_KEY_OK ret = %d\n", ret);

		if(ret >= 0)
		{
			status = ULINK_ERR_NONE;
			break;
		}
	}

	sprintf(outkey, "%08X%08X", key0, key1);

_err:
#if defined(__ANDROID__) || defined(TARGET_OS_IPHONE)
	ret = StopSmartConnection();
	if (ret != 0)
	{
		debug_log("StopSmartConnection error.\n");
	}
#endif
	return status;
}

/*
 * 连接到设备
 * devid 目标设备ID key 密钥 (均为8字节HEX字符串)
 * 返回ulink指针
 */
ULINK *ulink_open(const char *devid, const char *key)
{
	int i, ret, gethost;
	ULINK *ulink;
	UP2P_TOKEN stk;
	UP2P_HOST host;
	UP2P_DEVID devs;
	u32 cmd;

	ulink = malloc(sizeof(ULINK));
	if(ulink == NULL)
		return NULL;

	ret = ascii_to_2u32(devid, &ulink->dev0, &ulink->dev1);
	if(ret == -1)
		goto _err;

	ret = ascii_to_2u32(key, &ulink->key0, &ulink->key1);
	if(ret == -1)
		goto _err;

	ulink->sip = 0;
	ulink->sport = 0;

	gethost = 1;
	up2pa_clear_online(ulink->dev0, ulink->dev1);
	for(i = 0; i < ULINK_OPEN_TIMEOUT * 10; i++)
	{
		int online;

		// 发送检查请求
		up2pa_send_online(ulink->dev0, ulink->dev1);
		if(gethost)
		{
			up2pa_send_idx_cmd(0, 0, 0, CMD_GET_HOST, &devs, sizeof(UP2P_DEVID));
		}
		delay_ms(100);

		online = up2pa_check_online(ulink->dev0, ulink->dev1);
		if(online == 2)
		{
			// 局域网模式
			debug_log("switch to lan mode\n");
			break;
		}

		if(gethost)
		{
			ret = up2pa_recv_idx_cmd(0, 0, 0, &cmd, &host, sizeof(UP2P_HOST));
			if(ret == sizeof(UP2P_HOST) && cmd == CMD_GET_HOST_ACK)
			{
				// 更新服务器参数
				ulink->sip = host.ip;
				ulink->sport = host.port;
				// 设置服务器
				up2pa_set_host(ulink->sip, ulink->sport);
				debug_log("update_host to %x %x\n", ulink->sip, ulink->sport);
				gethost = 0;
				if(online == 1)
				{
					debug_log("switch to wan mode\n");
					break;
				}
			}
		}
	}

	if(i == ULINK_OPEN_TIMEOUT * 10)
	{
		debug_log("link timeout\n");
		goto _err;
	}

	// 每次一组通讯之前,要先刷新令牌,此处的作用是获取到令牌
	up2pa_send_data_cmd(ulink->dev0, ulink->dev1, ulink->key0, ulink->key1, 0, CMD_UPDATE_TOKEN, NULL, 0);
	ret = up2pa_wait_data_cmd(ulink->dev0, ulink->dev1, ulink->key0, ulink->key1, 0, CMD_UPDATE_TOKEN_ACK, &stk, sizeof(stk));
	debug_log("CMD_UPDATE_TOKEN_ACK ret = %d token = %x\n", ret, stk.token);
	if(ret == -1)
	{
		goto _err;
	}

	ulink->token = stk.token;

	return ulink;
_err:
	free(ulink);
	return NULL;
}

/*
 * 关闭连接
 * ulink ulink指针
 */
int ulink_close(ULINK *ulink)
{
	if(ulink == NULL)
		return -1;

	free(ulink);

	return 0;
}

/*
 * 发送命令
 * ulink ulink指针
 * cmd 命令字
 * param 附加数据
 * len 附加数据长度
 * 返回已发送字节数 -1 失败
 */
int ulink_cmd_send(ULINK *ulink, u32 cmd, void *param, int len)
{
	int ret;

	if(ulink == NULL)
		return -1;

	// 设置服务器
	//up2pa_set_host(ulink->sip, ulink->sport);

	ret = up2pa_send_data_cmd(ulink->dev0, ulink->dev1, ulink->key0, ulink->key1, ulink->token,
		cmd, param, len);

	return ret;
}

/*
 * 等待命令
 * ulink ulink指针
 * cmd 命令字
 * param 附加数据缓冲区
 * maxlen 缓冲区长度
 * 返回收到的附加数据长度 -1 失败
 */
int ulink_cmd_wait(ULINK *ulink, u32 cmd, void *param, int maxlen)
{
	int ret;

	if(ulink == NULL)
		return -1;

	ret = up2pa_wait_data_cmd(ulink->dev0, ulink->dev1, ulink->key0, ulink->key1, ulink->token, 
		cmd, param, maxlen);

	return ret;
}

/*
 * 发送检查在线状态的数据包, 查询在线状态之前先调用
 */
int ulink_send_online(ULINK *ulink)
{
	if(ulink == NULL)
		return -1;
	
	return up2pa_send_online(ulink->dev0, ulink->dev1);
}

/*
 * 检查在线状态
 * 返回值 0 不在线 1 广域网在线 2 局域网在线 -1 服务器无响应
 */
int ulink_check_online(ULINK *ulink)
{
	if(ulink == NULL)
		return -1;

	return up2pa_check_online(ulink->dev0, ulink->dev1);
}

/*
 * 边界保护开始
 * 在多线程中通讯序列使用此函数作为开始
 */
void ulink_border_begin()
{
	up2pa_border_begin();
}

/*
 * 边界保护结束
 * 在多线程中通讯序列使用此函数作为结束
 */
void ulink_border_end()
{
	up2pa_border_end();
}

/*
 * 调试信息输出, 0为关闭, 大于0为输出
 */
int ulink_debug(int level)
{
	up2pa_debug_level = level;
	return 0;
}

#ifdef  XMODE_SUP

/* 
This function calculates the CRC used by the "Modem Protocol" The first argument
is a pointer to the message block. The second argument is the number ofbytes
in the message block. The message block used by the Modem Protocol contains 
128 bytes. The function return value isan integer which contains the CRC.
The lo order 16 bits of this integer are the coefficients of the CRC. The 
The lo order bit is the lo order coefficient of the CRC. 
*/ 
static int calcrc(char *ptr, int count)
{ 
	int crc = 0, i; 
	while(--count >= 0) { 
		crc = crc ^ (int)*ptr++ << 8; 
		for(i = 0; i < 8; ++i) 
			if(crc & 0x8000) crc = crc << 1 ^ 0x1021; 
				else 
					crc = crc << 1; 
	} 
	return (crc & 0xFFFF); 
}
//文件是否已经导入内存
/*
int fileloadmen(void)
{
	int ret =-1;
	if( filebuff != NULL && filesize >0)
	{
		filecurr = 0;
		ret =0;
	}
	return ret;
}
//打开固件 开始传输
static int XmodeTxStar(char ackbuf,char *fname)
{
//	int ret =-1;
//	int count =0;
//	char buff[250];
	if((ackbuf != 'c' && ackbuf != 'C') ||fileloadmen() < 0){
		debug_log("ackbuf = %c\n",ackbuf);
		return -1;
		}
	return 0;
}
*/
//读取缓冲 / 文件的128b 并计算crc  存入缓存pbuf
//输出已经读取文件长度
//filebuff 文件缓冲  pbuf 发送缓冲
static int fread_crc(u8 *filebuff,char *pbuf,long filesize,long *filecurr,u8 pack_inc)
{
	int ret =-1;
	int llen; // let len
	int *crc_code;
//static u8 pack_red =0xfe;
//	u8 temp[150];
	Xmode_pack *xp =(Xmode_pack *)pbuf;

	int crc_temp;

	xp->hearder_cmd = XMOD_SOH ;
	xp->packernumber_rdc = ~pack_inc;
	xp->packernumber_inc = pack_inc;
	
	ret = filesize - (*filecurr);
		//读取文件的128b
	if( ret > TXBUFSIZE )
	{
		ret = TXBUFSIZE;
	}
	else	
	{
		if( ret == 0 )//读取到文件末尾
		{
			return ret;
		}
	}

	memset(xp->data,0x1A,TXBUFSIZE);
	memcpy(xp->data,&filebuff[(*filecurr)],ret);

	(*filecurr) += ret;

//	llen = TXBUFSIZE - ret;
//	if(llen>0)							//不足128B部分 用0x1a 填充
//		memset(&(xp->data[ret]),0x1A , llen);

	//crc 校验	
	crc_code = (int *)&(xp->data[TXBUFSIZE]); 
	crc_temp = calcrc((char *)xp->data,TXBUFSIZE);	//添加crc
	(*crc_code) = crc_temp;
//    debug_log("crc_temp:%d;crc_code:%d,\n",crc_temp,(*crc_code));
//    debug_log("pbuf_add:%d;crc_code_add:%d,\n",pbuf,crc_code);
//	debug_log("up2pa pbuf[0] =%2X ;pbuf[1] =%2X ;pbuf[2] =%2X ;pbuf[3] =%2X ;pbuf[131] =%2X ;pbuf[132] =%2X ;pbuf[133] =%2X ;send = %d\n",\
//								pbuf[0],pbuf[1],pbuf[2],pbuf[3],pbuf[131],pbuf[132],pbuf[133],ret);
	return  (TXBUFSIZE+TXMODEBASE+TXMODECRCLEN);	   // heard + date + crc_len
}
//填充 eot 终结符数据到buf
static int creat_eot(char *pbuf)
{
	int ret =-1;
	int *crc_code;
	static u8 pack_inc =0;		//包的序号
	static u8 pack_red =0xff;
	Xmode_pack *xp =(Xmode_pack *)pbuf;
 
	xp->hearder_cmd = XMOD_EOT ;
	//xp->packernumber_inc = pack_inc++;
	//xp->packernumber_rdc = pack_red--;
	
	return  (TXMODEBASE);	   //加上协议头长度
}

//while  发送更新包
/*
*ackbuf : 客户端回应CMD_WIFIUPDATE_TRI 的UP2P_PACKET->payload[0] ；ＵP2P_PACKET->payload[0] =="c" 则握手成功
*filebuff :固件在内存的buff首地址
*return : 0 成功 ；-1  失败
*/
//static u8 sendBuf[TXModMAXBUF+2];
int ulink_update_vm10x(ULINK *ulink, char *filebuff,u32 ulfilesize)
{

	int ret=-1;

	int packno=0;
	int sendpak=0;
	
	static 	u8 sendBuf[TXModMAXBUF+2];
	u8 xmode_txok_fig=0;
	
	u8 temrecBuf[MAX_SERIAL_SIZE];
	u8 rec_sta=XMOD_ACK;
	u8 retry_cnt = 0;  //重传计数器
	
	long foffset =0;
	long readlen = 0,freadlen=0;
	
	long filecurr = 0;			//当前文件位置
	u8 pack_inc =1;		//包的序号
	
    //  判断参数合法性
    if (filebuff==NULL || ulfilesize==0)
	{
		return -1;
	}
	//  判断参数合法性
    //握手
    ulink_cmd_send(ulink, CMD_WIFIUPDATE_TRI, NULL, 0);
    ret = ulink_cmd_wait(ulink, CMD_WIFIUPDATE_TRI_ACK, (char *)temrecBuf, MAX_SERIAL_SIZE);
    if((temrecBuf[0] != 'c' && temrecBuf[0] != 'C') )
	{
		debug_log("CMD_WIFIUPDATE_TRI_ACK ret = %d buf = %s\n", ret, temrecBuf);
		return -1;
	}

	//进入while  循环发 更新包
	while(1)
	{
	switch (rec_sta)
	{	
	case XMOD_ACK:			//读取文件128B + crc
		memset(sendBuf, 0, TXModMAXBUF);
		//发送128B 固件数据
		readlen = fread_crc(filebuff,(char *)sendBuf,ulfilesize,&filecurr,pack_inc);
		pack_inc++;//包的序号+1
		
		if(readlen == 0)
		{
			printf("done!!\n all update_SDK have been send to wifi.\n");
			printf("WIFI writting Flash... \n");
			sendBuf[0] = XMOD_EOT;
			readlen = 1;
			goto send_xmod;
		}
		//重置重发计数器
		retry_cnt =0;
		packno++;
		freadlen += readlen;
	case XMOD_NAK:
		//重发
		sendpak++;
		//重传判断 超出限制 则强行关闭
		if(++retry_cnt > MAXRETRANS )
		{
			temrecBuf[0] = XMOD_CAN;
			readlen = 1;
			goto EOT_END;
		}
send_xmod:
        ulink_cmd_send(ulink, CMD_WIFIUPDATE_DATA, (char *)sendBuf, readlen);
        memset(temrecBuf,0x00,5);
        ret = ulink_cmd_wait(ulink, CMD_WIFIUPDATE_DATA_ACK, (char *)temrecBuf, 5);

			//成功传输完成进入while  这是测试用
			//apk 端应该返回成功 return  0
		if(xmode_txok_fig == 1)
		{
			goto dowhile;
		}
		//终止传输
		if( temrecBuf[0] == XMOD_CAN &&temrecBuf[2] == XMOD_CAN)
		{
      		temrecBuf[0] = XMOD_CAN;
			goto EOT_END;
		}
		else if(temrecBuf[0] == XMOD_OK )
		{
			printf(" Successful, please Rebooting ...\n ");
			sendBuf[0] = XMOD_OK;
			readlen = 1;
			xmode_txok_fig = 1;
			goto send_xmod;
		}
		else if( temrecBuf[0] == XMOD_ACK &&temrecBuf[1] == pack_inc)		
		{//重置重传计数器
			debug_log("XMOD_ACK\n");
			readlen = 0;
			rec_sta = XMOD_ACK;
			printf("+");
		}
		else
		{
			debug_log("XMOD_NAK\n");
			rec_sta = XMOD_NAK;   //重传
			printf("-");
		}
		
		debug_log("up2pa pack_inc = %02x\n",pack_inc);
		debug_log("up2pa temrecBuf[0] = %02X\n",temrecBuf[0]);	
		debug_log("up2pa temrecBuf[1] = %02X\n",temrecBuf[1]);
		debug_log("up2pa temrecBuf[2] = %02X\n",temrecBuf[2]);
		debug_log("up2pa rec_sta = %d\n",rec_sta);
		debug_log("up2pa rece buf = %01X\n",sendBuf[0]);
		debug_log("up2pa freadlen = %d\n",freadlen);
		
		break;
	case XMOD_CAN:
		//终止传输
		temrecBuf[0] = XMOD_CAN;
		goto EOT_END;
		break; 
	default :
		break;
	}	
	

 	delay_ms(10);
	}
EOT_END:
		//设置接收符
//		readlen = creat_eot(temBuf);

        ulink_cmd_send(ulink, CMD_WIFIUPDATE_DATA, temrecBuf, 1);
        memset(temrecBuf,0x00,5);
		// 等待查询更新回应
 	 	ret = ulink_cmd_wait(ulink, CMD_WIFIUPDATE_DATA_ACK, (char *)temrecBuf, 5);
		//收到结束更新回应
	   /*	if(temBuf[0] == ACK)
			return freadlen;
			**/
		printf("\nUpdate failt !! please retry.\n");

		return -1;
		//debug_log("read pack :%d,send pack:%d \n",packno,sendpak);
		//debug_log("rec buff = %2X \n",sendBuf[0]);
        //debug_log("update end ret = %d\n",ret);

dowhile:
			
#ifdef _WIN32
	printf("restart up2pa.exe\n");
	return 0;//  升级ok
#elif __ANDROID__
	return 0;//  升级ok
#else
	while(1);
	return freadlen;
#endif
}

#endif



