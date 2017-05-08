/*******************************************************************************

    This file is part of the up2p.
    Copyright wilddog.com
    All right reserved.

    File:    up2p.h

    No description

    TIME LIST:
    CREATE  skyli   2017-05-06 13:46:05

*******************************************************************************/
#ifndef _up2p_h_
#define _up2p_h_

#ifdef		__cplusplus
extern "C" {
#endif

// ����ϵͳͷ�ļ�
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <winsock2.h>
#include <Mswsock.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define socklen_t int

#define Printf_High			printf

#else
#include <stdarg.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <pthread.h>
#ifndef __ANDROID__
#include <ifaddrs.h>
#endif
#include <netdb.h>
#define SOCKET int
typedef struct sockaddr SOCKADDR;
#define closesocket close
#endif

// �汾ʶ����
#define MAGIC 0x84F5AD94

// ���Ĵ������ݰ���С
#define MAX_SERIAL_SIZE 256

// ULINK���ӳ�ʱʱ��(��)
#define ULINK_OPEN_TIMEOUT	10

// wait�����ĳ�ʱʱ��(��)
#define MAX_WAIT_TIME 10

// ����ģ�����ʱ��(��)
#define MAX_CONFIG_TIME 120

// ����ʱ��(��)
#define UP2P_ONLINE_TIME 60
#define SYSTIMERUIN		 1000  //ϵͳʱ�䵥λs
//һ��sock ռ�õ��ڴ� ��λΪbytes
#define  SIZEOFSOCK   (sizeof(UDP_INFO))

#define UP2PC_PORT 27100
#define UP2PA_PORT 27101
#define UP2PS_PORT 27102

#define UP2PS_URL "www.wilddog.com"

/*	cmd**/
#define CMD_SEND_SERIAL_ACK_STRING     "ACK"
#define CMD_SERIAL_BUSY_STRING         "BUSY "

#define CMD_LAN_SCAN_ACK_STRING        "LAN "
#define CMD_SEND_KEY_STRING            "KEY "

// ��������
typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef long s32;
typedef unsigned long u32;

// ָ������
enum{
	// ����������
	CMD_NONE				= 0x0000,	// ������
	CMD_ONLINE				= 0x0010,	// ����ָ��
	CMD_ONLINE_ACK			= 0x0011,	// ���߻�Ӧ
	CMD_CHECK_ONLINE		= 0x0020,	// ����豸�Ƿ�����
	CMD_CHECK_ONLINE_YES	= 0x0021,	// ȷ������
	CMD_CHECK_ONLINE_NO		= 0x0022,	// ȷ�ϲ�����
	CMD_PING				= 0x0030,	// ���Ͳ���ָ��
	CMD_PING_ACK			= 0x0031,	// ���Ի�Ӧ
	CMD_LAN_SCAN			= 0x0040,	// ɨ��������豸
	CMD_LAN_SCAN_ACK		= 0x0041,	// ��Ӧɨ��
	CMD_GET_HOST			= 0x0050,	// ��ȡ��������ַ
	CMD_GET_HOST_ACK		= 0x0051,	// ���ط�������ַ

	// ��ʶ��������
	CMD_DATA				= 0x1000,	// ���ͼ�������
	CMD_DATA_ACK			= 0x1001,	// ��Ӧ��������
	CMD_DATA_KEY_ERR		= 0x1002,	// ��Ӧ��Կ�������ƴ���

	// �����ڲ�����
	CMD_UPDATE_TOKEN		= 0x2000,	// ˢ��token
	CMD_UPDATE_TOKEN_ACK	= 0x2001,	// ����ˢ�µ�token
	CMD_SEND_KEY			= 0x2010,	// �����µ�KEY���ͻ���
	CMD_SEND_KEY_OK			= 0x2011,	// �ͻ����Ѿ������µ�KEY
	CMD_CONFIG_HOST			= 0x2020,	// �������÷���������
	CMD_CONFIG_HOST_ACK		= 0x2021,	// ��Ӧ���÷����������ɹ�
	
	// �����û�����
	CMD_SEND_SERIAL			= 0x3000,	// ���ʹ�������
	CMD_SEND_SERIAL_ACK		= 0x3001,	// ���ʹ������ݻ�Ӧ
	CMD_READ_SERIAL			= 0x3010,	// ��ȡ��������
	CMD_READ_SERIAL_ACK		= 0x3011,	// ��ȡ�������ݻ�Ӧ
	CMD_TXRX_SERIAL			= 0x3020,	// ���շ�һ�����ʽ���ʹ�������
	CMD_TXRX_SERIAL_ACK		= 0x3021,	// ���շ�һ�����ʽ��Ӧ��������

 	CMD_WIFISDKVER_QURY		= 0x3030,	// ѯ��wifi sdk �İ汾��Ϣ
 	CMD_WIFISDKVER_QURY_ACK	= 0x3031,	// ѯ��wifi sdk �İ汾��Ϣ��Ӧ
	CMD_WIFIUPDATE_TRI		= 0x3040,	// ��������wifiSDK����
	CMD_WIFIUPDATE_TRI_ACK	= 0x3041,	// ��Ӧ����
	CMD_WIFIUPDATE_DATA		= 0x3050,	// ����wifi��������
	CMD_WIFIUPDATE_DATA_ACK	= 0x3051,	// ����wifi�������ݻ�Ӧ

	CMD_GPIO_INIT			= 0x3060,	// ����gpio
	CMD_GPIO_INIT_ACK		= 0x3061,	// ����gpio ��Ӧ
	CMD_GPIO_READ			= 0x3070,	// ��gpio
	CMD_GPIO_READ_ACK		= 0x3071,	// ��gpio ��Ӧ
	CMD_GPIO_WRITE			= 0x3080,	// дgpio
	CMD_GPIO_WRITE_ACK		= 0x3081,	// дgpio ��Ӧ
	CMD_PWM_INIT			= 0x3090,	// ����pwm
	CMD_PWM_INIT_ACK		= 0x3091,	// ����pwm ��Ӧ
	CMD_PWM_READ			= 0x30A0,	// ��pwm����
	CMD_PWM_READ_ACK		= 0x30A1,	// ��pwm������Ӧ
	CMD_PWM_WRITE			= 0x30B0,	// дpwm ����
	CMD_PWM_WRITE_ACK		= 0x30B1,	// дpwm ������Ӧ
	CMD_I2C_INIT			= 0x30C0,	// ����I2C
	CMD_I2C_INIT_ACK		= 0x30C1,	// I2C���û�Ӧ
	CMD_I2C_READ			= 0x30D0,	// ��ȡi2c����
	CMD_I2C_READ_ACK		= 0x30D1,	// ��ȡ��Ӧ
	CMD_I2C_WRITE			= 0x30E0,	// дi2c����
	CMD_I2C_WRITE_ACK		= 0x30E1,	// дi2c���ݻ�Ӧ
	CMD_I2C_TXRX			= 0x30F0,	// дi2c�ȴ�����
	CMD_I2C_TXRX_ACK		= 0x30F1,	// д��i2c���ݻ�Ӧ

	CMD_MAX
};

// ����Ϊ���Ĳ㶨��

// UDP��ز���
typedef struct{
	u32 sip;		// ԴIP
	u16 sport;		// Դ�˿�
	u32 dip;		// Ŀ��IP
	u16 dport;		// Ŀ��˿�
} UDP_INFO;

// �ܰ�
typedef struct{
	u32 magic;			// ��֤��,��ʾЭ���Э��汾
	u32 dst0;			// �豸��0
	u32 dst1;			// �豸��1��
	u32 src0;			// Դ�豸��0
	u32 src1;			// Դ�豸��1��Դ��Ŀ���豸�����ڰ����豸�Ŀ��ƹ���
	u32 cmd;			// ��������,�Ǽ�������
	u16 idx;			// ���
	u16 len;			// ���ݳ���
	char payload[0];	// ����
} UP2P_PACKET;

// ���ܴ�������ݰ�
typedef struct{
	u32 token;			// ����
	u32 cmd;			// ����
	u16 len;			// ���ݳ���
	char payload[0];	// ����
} UP2P_DATA;

// ��ȡ��������Ϣ���Ӱ�
typedef struct{
	u32 dev0;
	u32 dev1;
} UP2P_DEVID;

// ��������ַ��Ϣ
typedef struct{
	u32 ip;
	u16 port;
} UP2P_HOST;

// ����ΪӦ�ò㶨��

// ������Կ���ݰ�
typedef struct{
	u32 key0;
	u32 key1;
} UP2P_SET_KEY;

// �������ݰ�
typedef struct{
	u32 token;
} UP2P_TOKEN;

// ���÷�������Ϣ
typedef struct{
	u16 port;		// �������˿ں�
	char name[64];	// ������IP���������ַ���
} UP2P_CONFIG_HOST;

//gpio ����ģʽ
enum{
	//GPIO_=0,		//��ͨgpio �������
	GPIO_OUTPUT=0,
	GPIO_INPUT,
	GPIO_INTRTRI,	//�ж�����
	GPIO_PWM,		//pwmģʽ
	GPIO_I2C_SCL,   // i2cʱ��
	GPIO_I2C_SDA	//i2c���ݽ�
};

#define XMODE_SUP   1
#ifdef  XMODE_SUP
//x-mode Э��
#define TX128B		128
#define TX1024B		1024
#define TXBUFSIZE   128
#define TXMODEBASE	3 		//Э��ͷ ����
#define TXMODECRCLEN	4   //β��CRC ����

#define TXModMAXBUF	(TXBUFSIZE+ TXMODEBASE)
#define XMOD_SOH 0x01	//128���ݴ����ͷ��
#define XMOD_STX 0x02	//1024���ݴ���ͷ��
#define XMOD_EOT 0x04    //���Ͷ���ֹ����
#define XMOD_OK	 0X05	//XMOD ����ɹ�

#define XMOD_ACK 0x06    //��ȷ���ջ�Ӧ
#define XMOD_NAK 0x15    //�ط�����
#define XMOD_CAN 0x18    //���ն���ֹ����

#define DATAELEMENT	1	//����Ԫ��
#define CTRLZ 0x1A
#define DLY_1S 1000
#define MAXRETRANS 25
#define PACKET_RETRY_LIMIT 25

#define ISUPDATEINGSTAR()   updatestar_wifi()//��ʼ����
#define ISUPDATEING()   	updateing_wifi()//������
#define ISRUNINGAPCODE() 	isrunning_APcode()



//xmode ��
typedef struct{
	u8 hearder_cmd;			//�����ͷ��   				Byte1
	u8 packernumber_inc;	//�����0~++FF~0				Byte2
	u8 packernumber_rdc;	//����Ų��� FF~--~0~FF~--			Byte3
	u8 data[0];				//���� 128B   					Byte4 ~Byte131
	//int crc_code;			//crc  jУ����					Byte132~Byte133
}Xmode_pack;

//gpio 
// GPIO ���ݽṹ��
typedef struct{
	u8 pin;		// gpio pin �ź�
	u8 mode;	// gpio ����ģʽ������/�����
	u8 value;	// gpio pin �ŵ�ǰ��ƽֵ
} UP2P_GPIO;
enum
{
	eGPIO_Output = 0,//  ����ģʽ
	eGPIO_Input,	//  ���ģʽ

};

enum
{
	ePin0 = 0,	// pin0 Input/Output	I2C_CLK/GPIO/PWM	
	ePin1,		// pin1 Input/Output	I2C_SDA/GPIO/PWM	
	ePin2,		// pin2 Input	�����ָ���������/�̰���ʾ��ͨ�İ���	
	ePin3,		// pin3 Output	WIFI״ָ̬ʾ�ƣ�ϵͳԤ����	�޷�����
	ePin4,		// pin4 Input/Output	GPIO/PWM	
};
#define LOW_GPIO_LEV  	0
#define HIGHT_GPIO_LEV  1

typedef struct {

	u8 	pin;
	u16 timer_cnt;	 //��ʱ��
	u16	duty;		//ռ�ձ�
	u16 freq;		//Ƶ��
}pwm_cust_;

typedef struct {
	u8 clk_pin;	// i2c scl pin �ź�
	u8 sda_pin;	// i2c sda pin �ź�
	u8 speed;	// i2c �ٶȣ�1,10,40,��λ10k
	u8 address;	// i2c ���豸��ַ
	u8 w_len;   // д���ݳ���
	u8 r_len;   // �����ݳ���
	u8 date[0]; // ��/д��������

}I2C_cust_;


//�ⲿ����
int Xmodem_Update_FW(u8 const *xbuff,u8 *txbuff);
int updatewifi_int(void);
int  updatestar_wifi(void);
int  updateing_wifi(void);
int setupdateing_wifi(void);
int updatewifistar_int(void);
int isrunning_APcode(void);


#else 
#define ISUPDATEING()   0
#endif //xmode end
void delay_ms(u32 ms);

int debug_log(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
