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
 * ��ʼ����
 * host ��������������IP��ַ
 * appid ������Ψһ��ID�� 8�ֽ�HEX�ַ���
 * ����0�ɹ�
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
 * ж�ؿ�
 */
int ulink_deinit()
{
	up2pa_deinit();

	return 0;
}

static int gen_key(u32 *key0, u32 *key1)
{
	// ����key
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
 * �����豸
 * devid 16�ֽڵ��豸ID�ַ���
 * ssid ��Ҫ���ӵ��Ľ����SSID
 * key ��������Կ
 * type ��������
 * ����ֵ ULINK_ERR ���ҽ�������ULINK_ERR_NONEʱ��ʾ�ɹ�
 * outkey ������Կ�ַ���
 */
int ulink_config(const char *devid, const char *ssid, const char *key, int type, char *outkey)
{
	u32 token, dev0, dev1, key0, key1;
	int ret, i, status;
	UP2P_TOKEN stk;
	UP2P_HOST host;
    UP2P_CONFIG_HOST cfgh;

	// ����wifi
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

	// ����key
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
	// ��ȡ��������Ϣ
	up2pa_send_cmd(dev0, dev1, CMD_GET_HOST, NULL, 0);
	ret = up2pa_wait_cmd(dev0, dev1, CMD_GET_HOST_ACK, &host, sizeof(UP2P_HOST));
	if(ret >= 0)
	{
		// �з�������Ϣ�򱣴�
		up2pa_set_host(host.ip, host.port);
	}
#endif
	
	// ��ʼ��,���ÿͻ�����Կ
	for(i = 0; i < MAX_CONFIG_TIME; i++)
	{
		// �������״̬
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

		// ��ȡtoken
		// ���ƻ�ȡ�ɹ����ܽ��м���ͨѶ
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
		// ���÷�������ַ
		strcpy(cfgh.name, server_name);
		cfgh.port = htons(UP2PS_PORT);

		up2pa_send_data_cmd(dev0, dev1, 0, 0, token, CMD_CONFIG_HOST, &cfgh, sizeof(UP2P_CONFIG_HOST));
		// �ȴ�ACK
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
		// ����key
		up2pa_send_data_cmd(dev0, dev1, 0, 0, token, CMD_SEND_KEY, (const char *)&key, sizeof(UP2P_SET_KEY));
		// �ȴ�ACK
		ret = up2pa_wait_data_cmd(dev0, dev1, key0, key1, token, CMD_SEND_KEY_OK, NULL, 0);

		debug_log("CMD_SEND_KEY_OK ret = %d\n", ret);

		if(ret >= 0)
		{
			status = ULINK_ERR_NONE;
			break;
		}

		// ����key
		up2pa_send_data_cmd(dev0, dev1, key0, key1, token, CMD_SEND_KEY, (const char *)&key, sizeof(UP2P_SET_KEY));
		// �ȴ�ACK
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
 * ���ӵ��豸
 * devid Ŀ���豸ID key ��Կ (��Ϊ8�ֽ�HEX�ַ���)
 * ����ulinkָ��
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

		// ���ͼ������
		up2pa_send_online(ulink->dev0, ulink->dev1);
		if(gethost)
		{
			up2pa_send_idx_cmd(0, 0, 0, CMD_GET_HOST, &devs, sizeof(UP2P_DEVID));
		}
		delay_ms(100);

		online = up2pa_check_online(ulink->dev0, ulink->dev1);
		if(online == 2)
		{
			// ������ģʽ
			debug_log("switch to lan mode\n");
			break;
		}

		if(gethost)
		{
			ret = up2pa_recv_idx_cmd(0, 0, 0, &cmd, &host, sizeof(UP2P_HOST));
			if(ret == sizeof(UP2P_HOST) && cmd == CMD_GET_HOST_ACK)
			{
				// ���·���������
				ulink->sip = host.ip;
				ulink->sport = host.port;
				// ���÷�����
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

	// ÿ��һ��ͨѶ֮ǰ,Ҫ��ˢ������,�˴��������ǻ�ȡ������
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
 * �ر�����
 * ulink ulinkָ��
 */
int ulink_close(ULINK *ulink)
{
	if(ulink == NULL)
		return -1;

	free(ulink);

	return 0;
}

/*
 * ��������
 * ulink ulinkָ��
 * cmd ������
 * param ��������
 * len �������ݳ���
 * �����ѷ����ֽ��� -1 ʧ��
 */
int ulink_cmd_send(ULINK *ulink, u32 cmd, void *param, int len)
{
	int ret;

	if(ulink == NULL)
		return -1;

	// ���÷�����
	//up2pa_set_host(ulink->sip, ulink->sport);

	ret = up2pa_send_data_cmd(ulink->dev0, ulink->dev1, ulink->key0, ulink->key1, ulink->token,
		cmd, param, len);

	return ret;
}

/*
 * �ȴ�����
 * ulink ulinkָ��
 * cmd ������
 * param �������ݻ�����
 * maxlen ����������
 * �����յ��ĸ������ݳ��� -1 ʧ��
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
 * ���ͼ������״̬�����ݰ�, ��ѯ����״̬֮ǰ�ȵ���
 */
int ulink_send_online(ULINK *ulink)
{
	if(ulink == NULL)
		return -1;
	
	return up2pa_send_online(ulink->dev0, ulink->dev1);
}

/*
 * �������״̬
 * ����ֵ 0 ������ 1 ���������� 2 ���������� -1 ����������Ӧ
 */
int ulink_check_online(ULINK *ulink)
{
	if(ulink == NULL)
		return -1;

	return up2pa_check_online(ulink->dev0, ulink->dev1);
}

/*
 * �߽籣����ʼ
 * �ڶ��߳���ͨѶ����ʹ�ô˺�����Ϊ��ʼ
 */
void ulink_border_begin()
{
	up2pa_border_begin();
}

/*
 * �߽籣������
 * �ڶ��߳���ͨѶ����ʹ�ô˺�����Ϊ����
 */
void ulink_border_end()
{
	up2pa_border_end();
}

/*
 * ������Ϣ���, 0Ϊ�ر�, ����0Ϊ���
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
//�ļ��Ƿ��Ѿ������ڴ�
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
//�򿪹̼� ��ʼ����
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
//��ȡ���� / �ļ���128b ������crc  ���뻺��pbuf
//����Ѿ���ȡ�ļ�����
//filebuff �ļ�����  pbuf ���ͻ���
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
		//��ȡ�ļ���128b
	if( ret > TXBUFSIZE )
	{
		ret = TXBUFSIZE;
	}
	else	
	{
		if( ret == 0 )//��ȡ���ļ�ĩβ
		{
			return ret;
		}
	}

	memset(xp->data,0x1A,TXBUFSIZE);
	memcpy(xp->data,&filebuff[(*filecurr)],ret);

	(*filecurr) += ret;

//	llen = TXBUFSIZE - ret;
//	if(llen>0)							//����128B���� ��0x1a ���
//		memset(&(xp->data[ret]),0x1A , llen);

	//crc У��	
	crc_code = (int *)&(xp->data[TXBUFSIZE]); 
	crc_temp = calcrc((char *)xp->data,TXBUFSIZE);	//���crc
	(*crc_code) = crc_temp;
//    debug_log("crc_temp:%d;crc_code:%d,\n",crc_temp,(*crc_code));
//    debug_log("pbuf_add:%d;crc_code_add:%d,\n",pbuf,crc_code);
//	debug_log("up2pa pbuf[0] =%2X ;pbuf[1] =%2X ;pbuf[2] =%2X ;pbuf[3] =%2X ;pbuf[131] =%2X ;pbuf[132] =%2X ;pbuf[133] =%2X ;send = %d\n",\
//								pbuf[0],pbuf[1],pbuf[2],pbuf[3],pbuf[131],pbuf[132],pbuf[133],ret);
	return  (TXBUFSIZE+TXMODEBASE+TXMODECRCLEN);	   // heard + date + crc_len
}
//��� eot �ս�����ݵ�buf
static int creat_eot(char *pbuf)
{
	int ret =-1;
	int *crc_code;
	static u8 pack_inc =0;		//�������
	static u8 pack_red =0xff;
	Xmode_pack *xp =(Xmode_pack *)pbuf;
 
	xp->hearder_cmd = XMOD_EOT ;
	//xp->packernumber_inc = pack_inc++;
	//xp->packernumber_rdc = pack_red--;
	
	return  (TXMODEBASE);	   //����Э��ͷ����
}

//while  ���͸��°�
/*
*ackbuf : �ͻ��˻�ӦCMD_WIFIUPDATE_TRI ��UP2P_PACKET->payload[0] ����P2P_PACKET->payload[0] =="c" �����ֳɹ�
*filebuff :�̼����ڴ��buff�׵�ַ
*return : 0 �ɹ� ��-1  ʧ��
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
	u8 retry_cnt = 0;  //�ش�������
	
	long foffset =0;
	long readlen = 0,freadlen=0;
	
	long filecurr = 0;			//��ǰ�ļ�λ��
	u8 pack_inc =1;		//�������
	
    //  �жϲ����Ϸ���
    if (filebuff==NULL || ulfilesize==0)
	{
		return -1;
	}
	//  �жϲ����Ϸ���
    //����
    ulink_cmd_send(ulink, CMD_WIFIUPDATE_TRI, NULL, 0);
    ret = ulink_cmd_wait(ulink, CMD_WIFIUPDATE_TRI_ACK, (char *)temrecBuf, MAX_SERIAL_SIZE);
    if((temrecBuf[0] != 'c' && temrecBuf[0] != 'C') )
	{
		debug_log("CMD_WIFIUPDATE_TRI_ACK ret = %d buf = %s\n", ret, temrecBuf);
		return -1;
	}

	//����while  ѭ���� ���°�
	while(1)
	{
	switch (rec_sta)
	{	
	case XMOD_ACK:			//��ȡ�ļ�128B + crc
		memset(sendBuf, 0, TXModMAXBUF);
		//����128B �̼�����
		readlen = fread_crc(filebuff,(char *)sendBuf,ulfilesize,&filecurr,pack_inc);
		pack_inc++;//�������+1
		
		if(readlen == 0)
		{
			printf("done!!\n all update_SDK have been send to wifi.\n");
			printf("WIFI writting Flash... \n");
			sendBuf[0] = XMOD_EOT;
			readlen = 1;
			goto send_xmod;
		}
		//�����ط�������
		retry_cnt =0;
		packno++;
		freadlen += readlen;
	case XMOD_NAK:
		//�ط�
		sendpak++;
		//�ش��ж� �������� ��ǿ�йر�
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

			//�ɹ�������ɽ���while  ���ǲ�����
			//apk ��Ӧ�÷��سɹ� return  0
		if(xmode_txok_fig == 1)
		{
			goto dowhile;
		}
		//��ֹ����
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
		{//�����ش�������
			debug_log("XMOD_ACK\n");
			readlen = 0;
			rec_sta = XMOD_ACK;
			printf("+");
		}
		else
		{
			debug_log("XMOD_NAK\n");
			rec_sta = XMOD_NAK;   //�ش�
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
		//��ֹ����
		temrecBuf[0] = XMOD_CAN;
		goto EOT_END;
		break; 
	default :
		break;
	}	
	

 	delay_ms(10);
	}
EOT_END:
		//���ý��շ�
//		readlen = creat_eot(temBuf);

        ulink_cmd_send(ulink, CMD_WIFIUPDATE_DATA, temrecBuf, 1);
        memset(temrecBuf,0x00,5);
		// �ȴ���ѯ���»�Ӧ
 	 	ret = ulink_cmd_wait(ulink, CMD_WIFIUPDATE_DATA_ACK, (char *)temrecBuf, 5);
		//�յ��������»�Ӧ
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
	return 0;//  ����ok
#elif __ANDROID__
	return 0;//  ����ok
#else
	while(1);
	return freadlen;
#endif
}

#endif



