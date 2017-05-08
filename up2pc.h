/*******************************************************************************

    This file is part of the up2p.
    Copyright wilddog.com
    All right reserved.

    File:    up2pc.h

    No description

    TIME LIST:
    CREATE  skyli   2017-05-06 13:47:55

*******************************************************************************/
#ifndef _up2pc_h_
#define _up2pc_h_

#ifdef		__cplusplus
extern "C" {
#endif

#include "up2p.h"
#include "./up2pc/cust_config.h"
//DEBUG INFO
#ifdef _WIN32
#define Printf_High  printf
#else 


#endif 
#define ISUPDATEINGSTAR()   (updatestar_wifi())//��ʼ����
#define ISUPDATEING()   	(updateing_wifi())//������
//#define ISRUNINGAPCODE() 	( isrunning_APcode())
#define VER_ESP_SDK    "VM101_HQ1.5.02"  //20140820
	
//------------------�ڲ�����-----------------
/*
 * �������ݴ�����
 */
int up2pc_recv(const char *data, int len, UDP_INFO *info);

/*
 * ��ʱ��������״̬�Ļص�����
 * ����30�����һ��
 */
int up2pc_set_online();

// �����û���Կ
int up2pc_setkey(u32 key0, u32 key1);

/*
 * ��ѯ����״̬
 * ����0 ����
 * -1 ������
 */
int up2pc_get_online();

// ���÷�����ip�Ͷ˿�
int up2pc_set_server(u32 ip, u16 port);

// ���ñ���ip�Ͷ˿�
int up2pc_set_local(u32 ip, u16 port);

// ���ñ����豸��
int up2pc_set_name(u32 dev0, u32 dev1);

/*
 * ���·�������Ϣ
 * ��Ӧ�ò��ڳ�ʱ���޷�����ʱ����(�������½�����DNS֮��)
 * ֮ǰ�������ù�dev0 dev1
 */
int up2pc_update_host();

// ���÷�������Ϣ
int sys_config_host(char *host, u16 port);

//--------------�ⲿ����-----------------------------

// UDP��������
int udp_send(const char *data, int len, UDP_INFO *info);

// ������д����
int serial_write(const char *data, int len);

// �Ӵ��ڶ�����
int serial_read(char *data, int len);

// ϵͳ��ȡʱ���(��ȷ����)
u32 sys_get_time();

// ϵͳ��ȡ32λ�����
u32 sys_get_random();

// ������Կ
int sys_set_key(u32 key0, u32 key1);
//�ⲿ����
int Xmodem_Update_FW(u8 const *xbuff,u8 *txbuff);
int updatewifi_int(void);
int  updatestar_wifi(void);
int  updateing_wifi(void);
int setupdateing_wifi(void);
int updatewifistar_int(void);
int creatnewbinfile(char const *filename);
int ascii_to_2u32(const char *devid, u32 *dev0, u32 *dev1);

#ifdef __cplusplus
}
#endif

#endif
