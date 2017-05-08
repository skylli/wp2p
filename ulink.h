/*******************************************************************************

    This file is part of the project.
    Copyright wilddog.com
    All right reserved.

    File:    ulink.h

    No description

    TIME LIST:
    CREATE  skyli   2014-08-27 13:17:06

*******************************************************************************/
#ifndef _ulink_h_
#define _ulink_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _up2p_h_

// ���Ĵ������ݰ���С
#define MAX_SERIAL_SIZE 256

typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef long s32;
typedef unsigned long u32;

void delay_ms(u32 ms);

// ָ������
enum{
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
};

//gpio ����ģʽ
enum{
	GPIO_=0,		//��ͨgpio �������
	GPIO_OUTPUT,
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
	epinMX,
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
#endif
#endif

typedef struct ULINK ULINK;

/*
 * ulink_config ����ֵ
 */
typedef enum{
	ULINK_ERR_NONE,					// �ɹ�
	ULINK_ERR_DEVID_INVILD,			// ��Ч��devid
	ULINK_ERR_DEV_OFFLINE,			// �豸������
	ULINK_ERR_SERVER_OFFLINE,		// ������������, ���߹������粻ͨ
	ULINK_ERR_INIT_TOKEN,			// ��ʼ�����ƴ���, ������ģ���Ѿ������ù�
	ULINK_ERR_CONFIG_HOST,			// ���÷���������, ������ģ�����粻�û���flashдʧ��
	ULINK_ERR_INIT_KEY,				// ��ʼ����Կ����, ������ģ�����粻��
} ULINK_ERR;

/*
 * ��ʼ����
 * host ��������������IP��ַ
 * appid ������Ψһ��ID�� 16�ֽ�HEX�ַ���
 * ����0�ɹ�
 */
int ulink_init(const char *host, const char *appid);

/*
 * ж�ؿ�
 */
int ulink_deinit();

// WLAN��������
// �����ܵķ�ʽʹ��"ULINK_AUTH_OPENWEP", key����ΪNULL
#define ULINK_AUTH_OPENWEP			0
#define ULINK_AUTH_SHAREKEY			1
#define ULINK_AUTH_AUTOSWITCH		2
#define ULINK_AUTH_WPA				3
#define ULINK_AUTH_WPAPSK			4
#define ULINK_AUTH_WPANONE			5
#define ULINK_AUTH_WPA2				6
#define ULINK_AUTH_WPA2PSK			7
#define ULINK_AUTH_WPA1WPA2			8
#define ULINK_AUTH_WPA1PSKWPS2PSK	9
/*
 * �����豸
 * devid 16�ֽڵ��豸ID�ַ���
 * ssid ��Ҫ���ӵ��Ľ����SSID
 * key ��������Կ
 * type ��������
 * ����ֵ ULINK_ERR ���ҽ�������ULINK_ERR_NONEʱ��ʾ�ɹ�
 * outkey ������Կ�ַ���
 */
int ulink_config(const char *devid, const char *ssid, const char *key, int type, char *outkey);

/*
 * ���ӵ��豸
 * devid Ŀ���豸ID key ��Կ (��Ϊ8�ֽ�HEX�ַ���)
 * ����ulinkָ��
 */
ULINK *ulink_open(const char *devid, const char *key);

/*
 * �ر�����
 * ulink ulinkָ��
 */
int ulink_close(ULINK *ulink);

/*
 * ��������
 * ulink ulinkָ��
 * cmd ������
 * param ��������
 * len �������ݳ���
 * �����ѷ����ֽ��� -1 ʧ��
 */
int ulink_cmd_send(ULINK *ulink, u32 cmd, void *param, int len);

/*
 * �ȴ�����
 * ulink ulinkָ��
 * cmd ������
 * param �������ݻ�����
 * maxlen ����������
 * �����յ��ĸ������ݳ��� -1 ʧ��
 */
int ulink_cmd_wait(ULINK *ulink, u32 cmd, void *param, int maxlen);

/*
 * ���ͼ������״̬�����ݰ�, ��ѯ����״̬֮ǰ�ȵ���
 */
int ulink_send_online(ULINK *ulink);

/*
 * �������״̬
 * ����ֵ 0 ������ 1 ���������� 2 ���������� -1 ����������Ӧ
 */
int ulink_check_online(ULINK *ulink);

/*
 * �߽籣����ʼ
 * �ڶ��߳���ͨѶ����ʹ�ô˺�����Ϊ��ʼ
 */
void ulink_border_begin();

/*
 * �߽籣������
 * �ڶ��߳���ͨѶ����ʹ�ô˺�����Ϊ����
 */
void ulink_border_end();

/*
 * ������Ϣ���, 0Ϊ�ر�, ����0Ϊ���
 */
int ulink_debug(int level);


#ifdef  XMODE_SUP
/* vm10x�̼���������*/
int ulink_update_vm10x(ULINK *ulink, char *filebuff,u32 ulfilesize);

#endif

#ifdef __cplusplus
}
#endif

#endif
