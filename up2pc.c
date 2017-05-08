/*******************************************************************************

    This file is part of the up2p.
    Copyright wilddog.com
    All right reserved.

    File:    up2pc.c

    No description

    TIME LIST:
    CREATE  skyli   2017-05-06 13:47:42

*******************************************************************************/

#ifdef _WIN32
#include "up2pc.h"
#include <stdio.h>
#include <memory.h>
#else		
#include "up2p.h"
#include <stdio.h>
#include <string.h>
#include "cust_uart.h"
#include "iot_api.h"
#endif
#include "up2p_crypt.h"

// �û�����
static u32 up2pc_key0, up2pc_key1;
// ����������
static u32 server_ip;
static u16 server_port;
// ��������
static u32 local_ip;
static u16 local_port;
static u32 up2pc_dev0;
static u32 up2pc_dev1;

// ����ʱ���
static u32 online_time;
static u32 global_token;
static u16 token_idx;

u8 *p_mallocB = NULL;
#ifdef _WIN32
u8 *p_mallocA = NULL;
#endif

// ���ͻ���,�������һ����������
static char send_buf[256];

// ���֧�ֵ�APPȺ������
#define MAX_APPS 8

// APP����Ϣ��¼, ����������Ҫ
typedef struct{
	u32 mac0;
	u32 mac1;
	u32 ip;
	u16 port;
	u16 time;
} APPINFO;

static APPINFO appinfo[MAX_APPS];

#ifdef _WIN32
#else		
// ϵͳ��ȡʱ���(��ȷ����)
u32 sys_get_time(){
	u32 currenttimer;
	GET_CURRENT_SYSTIME(&currenttimer);

	return (currenttimer / SYSTIMERUIN);
}
#endif

/*
 * ��������
 */
static int up2pc_send(const char *data, int len, UDP_INFO *info)
{
	int ret;

	ret = udp_send(data, len, info);
	if(ret != len)
		return -1;

	return 0;
}

/*
 * ��Ӧ����
 * UDP_INFO��Դ��Ŀ�꽻��,���ͳ�ȥ
 */
static int up2pc_ack(const char *data, int len, UDP_INFO *info)
{
	UDP_INFO ack;

	ack.dip = info->sip;
	ack.dport = info->sport;
	ack.sip = info->dip;
	ack.sport = info->dport;

	up2pc_send(data, len, &ack);

	return 0;
}

// ���������
static int up2pc_send_cmd(UDP_INFO *info, u32 dst0, u32 dst1, u32 cmd, u16 idx, const char *param, int len)
{
	UP2P_PACKET *packet = (UP2P_PACKET *)send_buf;

	packet->magic = MAGIC;
	packet->src0 = up2pc_dev0;
	packet->src1 = up2pc_dev1;
	packet->dst0 = dst0;
	packet->dst1 = dst1;
	packet->cmd = cmd;
	packet->idx = idx;
	packet->len = len;

#ifdef _WIN32
	if(len > 0)
		memcpy(packet->payload, param, len);
#else		
	if(packet->len > 0 && packet->len<255)
	{
		memcpy(packet->payload, param, len);
	}
	else if( packet->len<0)
	{
		Printf_High("LEN <0!!!!\n");
		return -1;
	}    
	UDPDEBUG(("up2pc_send_cmd...\n"));
	UDPDEBUG(("packet...\n"));
#endif

	return up2pc_send((const char *)packet, sizeof(UP2P_PACKET) + len, info);
}

// �ظ������
static int up2pc_ack_cmd(UDP_INFO *info, u32 dst0, u32 dst1, u32 cmd, u16 idx, const char *param, int len)
{
	UDP_INFO ack;

	ack.dip = info->sip;
	ack.dport = info->sport;
	ack.sip = info->dip;
	ack.sport = info->dport;

	return up2pc_send_cmd(&ack, dst0, dst1, cmd, idx, param, len);
}

// �ظ��������ݰ�
static int up2pc_ack_data_cmd(UDP_INFO *info, u32 dst0, u32 dst1, u32 cmd, u16 idx, const char *param, int len)
{

	
	char *pkbuf = NULL;
	char *encbuf = NULL;

	UP2P_DATA *data = NULL;// (UP2P_DATA *)pkbuf;
	int enclen;
	int ret;
	u8 i;
	
	//+16 ���ܳ�����
	pkbuf = (char *)malloc(2*(sizeof(UP2P_DATA)+len +16));
	if(pkbuf == NULL)
		{
		Printf_High("malloc pkbuf failt;len = %d \n",(2 * sizeof(UP2P_DATA)+len +16));
		return -1;
	}
	encbuf = (char *)&pkbuf[(sizeof(UP2P_DATA)+len)];
/*
	encbuf = (char *)malloc(sizeof(UP2P_DATA)+len );
	if(encbuf == NULL)
		{
		Printf_High("malloc encbuf failt;len =%d\n",sizeof(UP2P_DATA)+len);
		return -1;
	}
*/	
	data = (UP2P_DATA *)pkbuf;//pkbuf;
	data->cmd = cmd;
	data->token = global_token;
	data->len = len;
	
//	memcpy(data->payload, param, len);
	for(i=0;i<len;i++)
		{
		data->payload[i] = param[i];
	}
/**/	
	//Printf_High(":len:%d;param::",len);
//  	Printf_High(":len:;param::");
//		usecDelay(1000);

//	for(i=0;i<len;i++)
//		Printf_High("%2X",param[i]);
/*	
	Printf_High("\n data->payload::");
	for(i=0;i<len;i++)
		Printf_High("%2X",data->payload[i]);
*/
	enclen = data_enc((char *)data, encbuf, (sizeof(UP2P_DATA) + len), up2pc_key0, up2pc_key1);
//	usecDelay(1000);

/*	
	Printf_High("\n encbuf::");
	for(i=0;i<len;i++)
		Printf_High("%2X",encbuf[sizeof(UP2P_DATA)+i]);
*/
/*
	if( pkbuf != NULL )
			{
			free(pkbuf);
			pkbuf = NULL; 
			}
*/
	ret = up2pc_ack_cmd(info, dst0, dst1, CMD_DATA_ACK, idx, encbuf, enclen);

	if( pkbuf != NULL )
			{
			free(pkbuf);
			pkbuf = NULL; 
			}

	return ret;
}
// ��������
static int up2pc_recv_cmd(UP2P_PACKET *inpkt, int len, UDP_INFO *info)
{
	UP2P_SET_KEY *key;
	u32 m_up2pc_key0 =0, m_up2pc_key1 =0;
	int ret = -1;
	int *pret=NULL;
	u8 i;
	u8 *pkbuf = NULL;//�ظ�֮��
	u8 *pBuf = NULL;
	u16 *freq = NULL ;
	u16 *duty = NULL;
	UP2P_PACKET *lastsend = (UP2P_PACKET *)send_buf;
	UP2P_PACKET *packet = NULL;
	UP2P_DATA *data = NULL;

#ifndef _WIN32  
	readserial_stc *pconn = &readserial;
	UIP_UDP_CONN *udp_conn = uip_udp_conn;
#endif	
	UP2P_CONFIG_HOST *chost = (UP2P_CONFIG_HOST *)data->payload;
	I2C_cust_ *pi2cbuf=NULL;
	UP2P_GPIO *pgpio_cust;
	pwm_cust_ *pwmcust=NULL;
    
	UP2P_TOKEN tmp;
	p_mallocB = (u8 *)malloc( sizeof(UP2P_PACKET) + len );
	if( p_mallocB == NULL )
	{
		Printf_High("p_mallocB failt!!\n");
		return ret;
		}
	packet = (UP2P_PACKET *)p_mallocB;
	data = (UP2P_DATA *)packet->payload;
	//Printf_High("recv_cmd::len = %d; up2pc_key0=%d;up2pc_key1=%d\n",len,up2pc_key0,up2pc_key1);
	memcpy(packet, inpkt, sizeof(UP2P_PACKET));

	// ����
  	data_dec(inpkt->payload, packet->payload, len, up2pc_key0, up2pc_key1);
	//�ͷſռ乩���溯��ʹ��
	if( p_mallocA != NULL )
			{
			free(p_mallocA);
			p_mallocA = NULL; 
			}
	Printf_High("cmd:%d \n",data->cmd);
//    Printf_High("up2pc_recv_cmd...\n");

	// ����ˢ�����Ƶ�����,��Ҫ��֤����
	if(data->cmd == CMD_UPDATE_TOKEN)
	{
		// ˢ������
		// �����ط�������,ʹ�����һ�ε�ֵ
		if(packet->idx == token_idx)
		{
			tmp.token = global_token;
		}
		else
		{
			tmp.token = sys_get_random();
			Printf_High("sys_get_random=%d\n",tmp.token);
		}
		token_idx = packet->idx;
		// ˢ�����Ƶ�ָ��ظ���token��0
		global_token = 0;
		ret = up2pc_ack_data_cmd(info, packet->src0, packet->src1,\
			CMD_UPDATE_TOKEN_ACK,packet->idx, (char *)&tmp, sizeof(UP2P_TOKEN));
		global_token = tmp.token;
		printf("update token to %x\n", global_token);
		return ret;
	}
	else
	{
		if(data->token != global_token)
		{
			// ��Ӧ���ƴ���
			ret = up2pc_ack_data_cmd(info, packet->src0, packet->src1,\
										CMD_DATA_KEY_ERR, packet->idx, NULL, 0);
			Printf_High("token error recv %4X != local %4x len = %d\n", data->token, global_token, data->len);
			return ret;
		}
	}

	// �����ط���ָ��, ֱ�ӻ�Ӧ
	if(lastsend->magic == MAGIC &&
		lastsend->idx == packet->idx &&
		lastsend->dst0 == packet->src0 &&
		lastsend->dst1 == packet->src1)
	{
		Printf_High("resend idx %d\n", packet->idx);
		return up2pc_send((const char *)lastsend, sizeof(UP2P_PACKET) + lastsend->len, info);
	}

	lastsend->dst0 = packet->dst0;
	lastsend->dst1 = packet->dst1;

	switch(data->cmd)
	{
	case CMD_SEND_SERIAL:
		// ��������
#ifdef _WIN32
		Sleep(500);
		serial_write(data->payload, data->len);
		// ��Ӧ
		up2pc_ack_data_cmd(info, packet->src0, packet->src1, CMD_SEND_SERIAL_ACK, packet->idx, NULL, 0);
#else
		Printf_High(("CMD_SEND_SERIAL...\n"));
		if(ISUPDATEING())
		{
			break;
		}
		// ��Ӧ
		if(	UDP_FREE())//no comm need to be  handle this time
		{
			up2pc_ack_data_cmd(info, packet->src0, packet->src1,\
								CMD_SEND_SERIAL_ACK, packet->idx, NULL, 0);
			serial_write(data->payload, data->len);
		}			
#endif
		break;
	case CMD_READ_SERIAL:
		// ��ȡ��������
#ifdef _WIN32
		Sleep(500);
		// ��ȡ��������
		ret = serial_read(data->payload, MAX_SERIAL_SIZE);
		if(ret > 0)
		{
			// ��Ӧ
			up2pc_ack_data_cmd(info, packet->src0, packet->src1,\
							CMD_READ_SERIAL_ACK, packet->idx, data->payload, ret);
		}
#else
		Printf_High(("CMD_READ_SERIAL...\n"));
		if(ISUPDATEING())
		{
			break;
		}
		if( uartrxle > 0 && 
			UDP_FREE() )//
		{   
			up2pc_ack_data_cmd(info, packet->src0, packet->src1,\
							 CMD_READ_SERIAL_ACK,packet->idx,uartrx_buff, uartrxle);
			uartrxle = 0;
		}else {
			up2pc_ack_data_cmd(info, packet->src0, packet->src1, \
								CMD_READ_SERIAL_ACK,packet->idx,uartrx_buff, 0);
		}
#endif
		break;
	case CMD_SEND_KEY:
#ifdef _WIN32
		key = (UP2P_SET_KEY *)data->payload;
		// ������Կ
		up2pc_key0 = key->key0;
		up2pc_key1 = key->key1;
#else
		if(ISUPDATEING()) 
		{
			break;
		}
	//	Printf_High("CMD_SEND_KEY...22\n");	
//		pBuf = (u8 *)data->payload;
//		m_up2pc_key0 = pBuf[0]|(pBuf[1]<<8)|(pBuf[2]<<16)|(pBuf[3]<<24);
//		m_up2pc_key1 = pBuf[4]|(pBuf[5]<<8)|(pBuf[6]<<16)|(pBuf[7]<<24);
//		up2pc_key0 = key->key0;
//up2pc_key1 = key->key1;
//		Printf_High("key0=%4X;key1 = %4X\n",m_up2pc_key0,m_up2pc_key1);
#endif
		if(sys_set_key(up2pc_key0, up2pc_key1) !=0 )
			break;
		
		up2pc_ack_data_cmd(info, packet->src0, packet->src1, CMD_SEND_KEY_OK, packet->idx, NULL, 0);
		break;

	case CMD_CONFIG_HOST:
		// ���÷�������Ϣ
		// ģ���ע��: ��ʹDNS�������ɹ�ҲҪ�������ݲ�����ACK
		// ����дflash����ʧ��,��Ҫ��ӦACK
		{
			UP2P_CONFIG_HOST *chost = (UP2P_CONFIG_HOST *)data->payload;
			if(sys_config_host(chost->name, chost->port) == 0)
			{
				// ���óɹ���Ӧ
				up2pc_ack_data_cmd(info, packet->src0, packet->src1, CMD_CONFIG_HOST_ACK, packet->idx, NULL, 0);
				break;
			}
		}
		break;

	case CMD_TXRX_SERIAL:
		// ��������
		serial_write(data->payload, data->len);
		// ��ȡ��������
		ret = serial_read(data->payload, MAX_SERIAL_SIZE);
		if(ret > 0)
		{
			// ��Ӧ
			up2pc_ack_data_cmd(info, packet->src0, packet->src1, CMD_TXRX_SERIAL_ACK, packet->idx, data->payload, ret);
		}

		
		break;
#ifdef  XMODE_SUP
	    case CMD_WIFISDKVER_QURY:
			// wifi sdk �汾��Ϣѯ��
			Printf_High("CMD_WIFISDKVER_QURY \n");
			
			if( !ISUPDATEINGSTAR() )
				break;

			up2pc_ack_data_cmd(info, packet->src0, packet->src1, CMD_WIFIUPDATE_TRI_ACK, packet->idx,\
															VER_ESP_SDK, sizeof(VER_ESP_SDK)-1);
			break;
		case CMD_WIFIUPDATE_TRI:
			// wifi sdk ��������
			Printf_High("CMD_WIFIUPDATE_TRI \n");
			
			if( !ISUPDATEINGSTAR() )
				break;
			//��ʼ�����µ����ݽṹ
			updatewifistar_int();
			pkbuf = (u8 *)malloc(1);
			if( pkbuf  == NULL)
				{
				Printf_High("pkbuf malloc failt! \n");
				break;
			}
			pkbuf[0] = 'c'; //��Ӧ
			up2pc_ack_data_cmd(info, packet->src0, packet->src1, CMD_WIFIUPDATE_TRI_ACK, packet->idx, pkbuf, 1);
			break;
		case CMD_WIFIUPDATE_DATA:
		 	Printf_High("CMD_WIFIUPDATE_DATA \n");
			//�յ�wifi �̼� ��������
			//�ֵ�һ��128b �Ĺ̼�����
			//��crc У�飬��������� 
			//д��flash�� 
			//����Ӧ

			//������
			if(!ISUPDATEING())
			{
				break;
			}
			//��ֹ������³�ʼ��
			setupdateing_wifi();
			
			pkbuf = (u8 *)malloc(4);
			if( pkbuf  == NULL)
				{
				Printf_High("pkbuf malloc failt! \n");
				break;
			}
			len = Xmodem_Update_FW(data->payload,pkbuf);
			
			up2pc_ack_data_cmd(info, packet->src0, packet->src1,\
								CMD_WIFIUPDATE_DATA_ACK, packet->idx, (const char*)&pkbuf[0], len);
			//Printf_High("pkbuf[0]:%02X;pkbuf[1]:%02X;pkbuf[2]:%02X;len",pkbuf[0],pkbuf[1],pkbuf[2],len);
			break;
			//gpio pwm i2c fuctions
		case CMD_GPIO_INIT:
			Printf_High("CMD_GPIO_READ \n");

			break;
			
		case CMD_GPIO_READ:
			Printf_High("CMD_GPIO_READ \n");
			break;
		case CMD_GPIO_WRITE:
			
			Printf_High("CMD_GPIO_WRITE \n");	
			break;
		case CMD_PWM_INIT:
			
			Printf_High("CMD_PWM_INIT \n");	
			break;
			
		case CMD_PWM_READ:
			Printf_High("CMD_PWM_READ \n");	
			break;
			
		case CMD_PWM_WRITE:
			
			Printf_High("CMD_PWM_WRITE \n");
			break;
		
		case CMD_I2C_INIT:
			Printf_High("CMD_I2C_INIT \n");
			break;
		case CMD_I2C_READ:
			Printf_High("CMD_I2C_READ \n");
			break;
		case CMD_I2C_WRITE:
			Printf_High("CMD_I2C_WRITE \n");
			break;
		case CMD_I2C_TXRX:		
			Printf_High("CMD_I2C_TXRX \n");
					
			break;
#endif //xmode

	}
	if( pkbuf!= NULL )
		{
			free(pkbuf);
			pkbuf = NULL; 
	}
	return ret;
}

/*
 * �������ݴ�����
 */
int up2pc_recv(const char *data, int len, UDP_INFO *info)
{
	UP2P_PACKET *packet = (UP2P_PACKET *)data;
	UP2P_HOST *host;
	int ret = -1;

#ifdef _WIN32
#else
//	UDPDEBUG(("up2pc_recv : data"));
#endif
	// ��֤��
	if(packet->magic != MAGIC)
	{
		Printf_High("error MAGIC\n");
		Printf_High("packet->magic =  %d\n",packet->magic);
		Printf_High("MAGIC = %d\n",MAGIC);	
 		Printf_High("up2pc_key0 = %04X,up2pc_key1 = %04X\n",up2pc_key0,up2pc_key1);	
		return -1;
	}
	
	//��֤dev
	if(packet->dst0 != up2pc_dev0 || packet->dst1 != up2pc_dev1)
	{
#ifdef _WIN32
#else
	//	Printf_High("dev  identify error !1packet->up2pc_dev0 != up2pc_dev0\n ");
#endif
		return -1;
	}
	
	switch(packet->cmd)
	{
		case CMD_ONLINE_ACK:
		// ��������Ӧ����
		online_time = sys_get_time();
#ifdef _WIN32
#else		
	    Printf_High(("CMD_ONLINE_ACK...\n"));
		if(ISUPDATEING())
					break;

		wilddogcnn.onlinetimeout_fig =0;
#endif
		ret = 0;
		break;

		case CMD_PING:
		// ��Ӧ����ָ��
#ifdef _WIN32
#else		
		Printf_High(("CMD_PING...\n"));
		if(ISUPDATEING())
				break;
#endif
		ret = up2pc_ack_cmd(info, packet->src0, packet->src1, CMD_PING_ACK, packet->idx, NULL, 0);
		break;

		case CMD_LAN_SCAN:
		// APP������ɨ������״̬
#ifdef _WIN32
#else
		Printf_High(("CMD_LAN_SCAN\n"));
		if(ISUPDATEING())
				break;
#endif
		ret = up2pc_ack_cmd(info, packet->src0, packet->src1, CMD_LAN_SCAN_ACK, packet->idx, NULL, 0);
		break;

		case CMD_GET_HOST_ACK:
		// ��ȡ��������Ϣ����
		host = packet->payload;
		local_ip = host->ip;
		local_port = host->port;
		printf("Update host %x %x\n", local_ip, local_port);
		break;

		case CMD_DATA:
			
		Printf_High(("CMD_DATA\n"));
		// ���ܵ�����
		ret = up2pc_recv_cmd(packet, len, info);
		
		if( p_mallocB!= NULL )
		{
			free(p_mallocB);
			p_mallocB = NULL; 
		}
		
		break;
	}

	return ret;
}

/*
 * ��ʱ��������״̬�Ļص�����
 * ����30�����һ��
 */
int up2pc_set_online()
{
	UDP_INFO info;

#ifdef _WIN32
	info.dip = server_ip;
	info.dport = server_port;
	info.sip = local_ip;
	info.sip = local_port;
#else
	//Printf_High("up2pc_set_online:onlinereport_fig:%d\n",onlinereport_fig);
	if(onlinereport_fig ==0 )
			return 0;
	Printf_High("DNS rport :%d\n",HTONS(uip_udp_conn->rport));
	
	printf("UDP connetion tx: lp:%d,rp:%d,  \n",			
			HTONS(uip_udp_conn->lport), HTONS(uip_udp_conn->rport));
	
	Printf_High("DNS IP, %d.%d.%d.%d\n",
					htons(uip_udp_conn->ripaddr[0]) >>8,  
					htons(uip_udp_conn->ripaddr[0]) & 0xff,  
					htons(uip_udp_conn->ripaddr[1]) >>8,  
					htons(uip_udp_conn->ripaddr[1]) & 0xff);
	onlinereport_fig = 0;
#endif	

	return up2pc_send_cmd(&info, 0, 0, CMD_ONLINE, 0, NULL, 0);
}

/*
 * ���·�������Ϣ
 * ��Ӧ�ò��ڳ�ʱ���޷�����ʱ����(�������½�����DNS֮��)
 * ֮ǰ�������ù�dev0 dev1
 */
int up2pc_update_host()
{
	UDP_INFO info;
	UP2P_DEVID devid;

	info.dip = server_ip;
	info.dport = server_port;
	info.sip = local_ip;
	info.sip = local_port;

	devid.dev0 = up2pc_dev0;
	devid.dev1 = up2pc_dev1;

	return up2pc_send_cmd(&info, 0, 0, CMD_GET_HOST, 0, &devid, sizeof(UP2P_DEVID));
}

// �����û���Կ
int up2pc_setkey(u32 key0, u32 key1)
{
	up2pc_key0 = key0;
	up2pc_key1 = key1;

	return 0;
}

/*
 * ��ѯ����״̬
 * ����0 ����
 * -1 ������
 */
int up2pc_get_online()
{
#ifdef _WIN32
#else
	Printf_High(".....sysgettime: %d;onlinetime= %d;",sys_get_time(),online_time);
#endif	
	if(sys_get_time() - online_time > UP2P_ONLINE_TIME)
		return -1;

	return 0;
}

// ���÷�����ip�Ͷ˿�
int up2pc_set_server(u32 ip, u16 port)
{
	server_ip = ip;
	server_port = port;

	return 0;
}

// ���ñ���ip�Ͷ˿�
int up2pc_set_local(u32 ip, u16 port)
{
	local_ip = ip;
	local_port = port;

	return 0;
}

// ���ñ����豸��
int up2pc_set_name(u32 dev0, u32 dev1)
{
	up2pc_dev0 = dev0;
	up2pc_dev1 = dev1;

	return 0;
}
//ascii conver to u32
int ascii_to_2u32(const char *devid, u32 *dev0, u32 *dev1)
{
	char tmp0[32];
	char tmp1[32];

	if(strlen(devid) != 16)
		return -1;

	strncpy(tmp0, devid, 8);
	strncpy(tmp1, devid + 8, 8);

	sscanf(tmp0, "%x", dev0);
	sscanf(tmp1, "%x", dev1);

	return 0;
}

