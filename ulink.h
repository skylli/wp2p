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

// 最大的串口数据包大小
#define MAX_SERIAL_SIZE 256

typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef long s32;
typedef unsigned long u32;

void delay_ms(u32 ms);

// 指令类型
enum{
	CMD_SEND_SERIAL			= 0x3000,	// 发送串口数据
	CMD_SEND_SERIAL_ACK		= 0x3001,	// 发送串口数据回应
	CMD_READ_SERIAL			= 0x3010,	// 读取串口数据
	CMD_READ_SERIAL_ACK		= 0x3011,	// 读取串口数据回应
	CMD_TXRX_SERIAL			= 0x3020,	// 以收发一体的形式发送串口数据
	CMD_TXRX_SERIAL_ACK		= 0x3021,	// 返收发一体的形式回应串口数据

 	CMD_WIFISDKVER_QURY		= 0x3030,	// 询问wifi sdk 的版本信息
 	CMD_WIFISDKVER_QURY_ACK	= 0x3031,	// 询问wifi sdk 的版本信息回应
	CMD_WIFIUPDATE_TRI		= 0x3040,	// 触发更新wifiSDK命令
	CMD_WIFIUPDATE_TRI_ACK	= 0x3041,	// 回应更新
	CMD_WIFIUPDATE_DATA		= 0x3050,	// 传输wifi更新数据
	CMD_WIFIUPDATE_DATA_ACK	= 0x3051,	// 传输wifi更新数据回应

	CMD_GPIO_INIT			= 0x3060,	// 配置gpio
	CMD_GPIO_INIT_ACK		= 0x3061,	// 配置gpio 回应
	CMD_GPIO_READ			= 0x3070,	// 读gpio
	CMD_GPIO_READ_ACK		= 0x3071,	// 读gpio 回应
	CMD_GPIO_WRITE			= 0x3080,	// 写gpio
	CMD_GPIO_WRITE_ACK		= 0x3081,	// 写gpio 回应
	CMD_PWM_INIT			= 0x3090,	// 配置pwm
	CMD_PWM_INIT_ACK		= 0x3091,	// 配置pwm 回应
	CMD_PWM_READ			= 0x30A0,	// 读pwm参数
	CMD_PWM_READ_ACK		= 0x30A1,	// 读pwm参数回应
	CMD_PWM_WRITE			= 0x30B0,	// 写pwm 参数
	CMD_PWM_WRITE_ACK		= 0x30B1,	// 写pwm 参数回应
	CMD_I2C_INIT			= 0x30C0,	// 配置I2C
	CMD_I2C_INIT_ACK		= 0x30C1,	// I2C配置回应
	CMD_I2C_READ			= 0x30D0,	// 读取i2c数据
	CMD_I2C_READ_ACK		= 0x30D1,	// 读取回应
	CMD_I2C_WRITE			= 0x30E0,	// 写i2c数据
	CMD_I2C_WRITE_ACK		= 0x30E1,	// 写i2c数据回应
	CMD_I2C_TXRX			= 0x30F0,	// 写i2c等待数据
	CMD_I2C_TXRX_ACK		= 0x30F1,	// 写读i2c数据回应
};

//gpio 控制模式
enum{
	GPIO_=0,		//普通gpio 输入输出
	GPIO_OUTPUT,
	GPIO_INPUT,
	GPIO_INTRTRI,	//中断输入
	GPIO_PWM,		//pwm模式
	GPIO_I2C_SCL,   // i2c时钟
	GPIO_I2C_SDA	//i2c数据脚
};

#define XMODE_SUP   1
#ifdef  XMODE_SUP
//x-mode 协议
#define TX128B		128
#define TX1024B		1024
#define TXBUFSIZE   128
#define TXMODEBASE	3 		//协议头 长度
#define TXMODECRCLEN	4   //尾部CRC 长度

#define TXModMAXBUF	(TXBUFSIZE+ TXMODEBASE)
#define XMOD_SOH 0x01	//128数据传输的头部
#define XMOD_STX 0x02	//1024数据传输头部
#define XMOD_EOT 0x04    //发送端终止传输
#define XMOD_OK	 0X05	//XMOD 传输成功

#define XMOD_ACK 0x06    //正确接收回应
#define XMOD_NAK 0x15    //重发请求
#define XMOD_CAN 0x18    //接收端终止传输

#define DATAELEMENT	1	//数据元素
#define CTRLZ 0x1A
#define DLY_1S 1000
#define MAXRETRANS 25
#define PACKET_RETRY_LIMIT 25

#define ISUPDATEINGSTAR()   updatestar_wifi()//开始更新
#define ISUPDATEING()   	updateing_wifi()//更新中
#define ISRUNINGAPCODE() 	isrunning_APcode()



//xmode 包
typedef struct{
	u8 hearder_cmd;			//传输的头部   				Byte1
	u8 packernumber_inc;	//包序号0~++FF~0				Byte2
	u8 packernumber_rdc;	//包序号补码 FF~--~0~FF~--			Byte3
	u8 data[0];				//数据 128B   					Byte4 ~Byte131
	//int crc_code;			//crc  j校验码					Byte132~Byte133
}Xmode_pack;

//gpio 
// GPIO 数据结构体
typedef struct{
	u8 pin;		// gpio pin 脚号
	u8 mode;	// gpio 工作模式（输入/输出）
	u8 value;	// gpio pin 脚当前电平值
} UP2P_GPIO;
enum
{
	eGPIO_Output = 0,//  输入模式
	eGPIO_Input,	//  输出模式

};

enum
{
	ePin0 = 0,	// pin0 Input/Output	I2C_CLK/GPIO/PWM	
	ePin1,		// pin1 Input/Output	I2C_SDA/GPIO/PWM	
	ePin2,		// pin2 Input	长按恢复出厂设置/短按表示普通的按键	
	ePin3,		// pin3 Output	WIFI状态指示灯（系统预留）	无法配置
	ePin4,		// pin4 Input/Output	GPIO/PWM	
	epinMX,
};
#define LOW_GPIO_LEV  	0
#define HIGHT_GPIO_LEV  1

typedef struct {
	u8 	pin;
	u16 timer_cnt;	 //计时器
	u16	duty;		//占空比
	u16 freq;		//频率
}pwm_cust_;

typedef struct {
	u8 clk_pin;	// i2c scl pin 脚号
	u8 sda_pin;	// i2c sda pin 脚号
	u8 speed;	// i2c 速度，1,10,40,单位10k
	u8 address;	// i2c 从设备地址
	u8 w_len;   // 写数据长度
	u8 r_len;   // 读数据长度
	u8 date[0]; // 读/写数据内容

}I2C_cust_;
#endif
#endif

typedef struct ULINK ULINK;

/*
 * ulink_config 返回值
 */
typedef enum{
	ULINK_ERR_NONE,					// 成功
	ULINK_ERR_DEVID_INVILD,			// 无效的devid
	ULINK_ERR_DEV_OFFLINE,			// 设备不在线
	ULINK_ERR_SERVER_OFFLINE,		// 服务器不在线, 或者广域网络不通
	ULINK_ERR_INIT_TOKEN,			// 初始化令牌错误, 可能是模块已经被配置过
	ULINK_ERR_CONFIG_HOST,			// 配置服务器错误, 可能是模块网络不好或者flash写失败
	ULINK_ERR_INIT_KEY,				// 初始化密钥错误, 可能是模块网络不好
} ULINK_ERR;

/*
 * 初始化库
 * host 服务器域名或者IP地址
 * appid 代表本机唯一的ID码 16字节HEX字符串
 * 返回0成功
 */
int ulink_init(const char *host, const char *appid);

/*
 * 卸载库
 */
int ulink_deinit();

// WLAN加密类型
// 不加密的方式使用"ULINK_AUTH_OPENWEP", key设置为NULL
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
 * 配置设备
 * devid 16字节的设备ID字符串
 * ssid 将要连接到的接入点SSID
 * key 接入点的密钥
 * type 加密类型
 * 返回值 ULINK_ERR 当且仅当返回ULINK_ERR_NONE时表示成功
 * outkey 返回密钥字符串
 */
int ulink_config(const char *devid, const char *ssid, const char *key, int type, char *outkey);

/*
 * 连接到设备
 * devid 目标设备ID key 密钥 (均为8字节HEX字符串)
 * 返回ulink指针
 */
ULINK *ulink_open(const char *devid, const char *key);

/*
 * 关闭连接
 * ulink ulink指针
 */
int ulink_close(ULINK *ulink);

/*
 * 发送命令
 * ulink ulink指针
 * cmd 命令字
 * param 附加数据
 * len 附加数据长度
 * 返回已发送字节数 -1 失败
 */
int ulink_cmd_send(ULINK *ulink, u32 cmd, void *param, int len);

/*
 * 等待命令
 * ulink ulink指针
 * cmd 命令字
 * param 附加数据缓冲区
 * maxlen 缓冲区长度
 * 返回收到的附加数据长度 -1 失败
 */
int ulink_cmd_wait(ULINK *ulink, u32 cmd, void *param, int maxlen);

/*
 * 发送检查在线状态的数据包, 查询在线状态之前先调用
 */
int ulink_send_online(ULINK *ulink);

/*
 * 检查在线状态
 * 返回值 0 不在线 1 广域网在线 2 局域网在线 -1 服务器无响应
 */
int ulink_check_online(ULINK *ulink);

/*
 * 边界保护开始
 * 在多线程中通讯序列使用此函数作为开始
 */
void ulink_border_begin();

/*
 * 边界保护结束
 * 在多线程中通讯序列使用此函数作为结束
 */
void ulink_border_end();

/*
 * 调试信息输出, 0为关闭, 大于0为输出
 */
int ulink_debug(int level);


#ifdef  XMODE_SUP
/* vm10x固件升级函数*/
int ulink_update_vm10x(ULINK *ulink, char *filebuff,u32 ulfilesize);

#endif

#ifdef __cplusplus
}
#endif

#endif
