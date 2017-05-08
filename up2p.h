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

// 公共系统头文件
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

// 版本识别码
#define MAGIC 0x84F5AD94

// 最大的串口数据包大小
#define MAX_SERIAL_SIZE 256

// ULINK连接超时时长(秒)
#define ULINK_OPEN_TIMEOUT	10

// wait函数的超时时长(秒)
#define MAX_WAIT_TIME 10

// 配置模块最大时长(秒)
#define MAX_CONFIG_TIME 120

// 在线时间(秒)
#define UP2P_ONLINE_TIME 60
#define SYSTIMERUIN		 1000  //系统时间单位s
//一个sock 占用的内存 单位为bytes
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

// 基本类型
typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef long s32;
typedef unsigned long u32;

// 指令类型
enum{
	// 不加密命令
	CMD_NONE				= 0x0000,	// 空类型
	CMD_ONLINE				= 0x0010,	// 在线指令
	CMD_ONLINE_ACK			= 0x0011,	// 在线回应
	CMD_CHECK_ONLINE		= 0x0020,	// 检查设备是否在线
	CMD_CHECK_ONLINE_YES	= 0x0021,	// 确认在线
	CMD_CHECK_ONLINE_NO		= 0x0022,	// 确认不在线
	CMD_PING				= 0x0030,	// 发送测试指令
	CMD_PING_ACK			= 0x0031,	// 测试回应
	CMD_LAN_SCAN			= 0x0040,	// 扫描局域网设备
	CMD_LAN_SCAN_ACK		= 0x0041,	// 回应扫描
	CMD_GET_HOST			= 0x0050,	// 获取服务器地址
	CMD_GET_HOST_ACK		= 0x0051,	// 返回服务器地址

	// 标识加密命令
	CMD_DATA				= 0x1000,	// 发送加密数据
	CMD_DATA_ACK			= 0x1001,	// 回应加密数据
	CMD_DATA_KEY_ERR		= 0x1002,	// 回应密钥或者令牌错误

	// 加密内部命令
	CMD_UPDATE_TOKEN		= 0x2000,	// 刷新token
	CMD_UPDATE_TOKEN_ACK	= 0x2001,	// 返回刷新的token
	CMD_SEND_KEY			= 0x2010,	// 发送新的KEY到客户端
	CMD_SEND_KEY_OK			= 0x2011,	// 客户端已经接受新的KEY
	CMD_CONFIG_HOST			= 0x2020,	// 发送配置服务器参数
	CMD_CONFIG_HOST_ACK		= 0x2021,	// 回应配置服务器参数成功
	
	// 加密用户命令
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

	CMD_MAX
};

// 以下为核心层定义

// UDP相关参数
typedef struct{
	u32 sip;		// 源IP
	u16 sport;		// 源端口
	u32 dip;		// 目标IP
	u16 dport;		// 目标端口
} UDP_INFO;

// 总包
typedef struct{
	u32 magic;			// 验证数,表示协议和协议版本
	u32 dst0;			// 设备名0
	u32 dst1;			// 设备名1，
	u32 src0;			// 源设备名0
	u32 src1;			// 源设备名1，源和目的设备号用于包的设备的控制管理
	u32 cmd;			// 命令类型,非加密命令
	u16 idx;			// 序号
	u16 len;			// 数据长度
	char payload[0];	// 数据
} UP2P_PACKET;

// 加密传输的数据包
typedef struct{
	u32 token;			// 令牌
	u32 cmd;			// 命令
	u16 len;			// 数据长度
	char payload[0];	// 数据
} UP2P_DATA;

// 获取服务器信息附加包
typedef struct{
	u32 dev0;
	u32 dev1;
} UP2P_DEVID;

// 服务器地址信息
typedef struct{
	u32 ip;
	u16 port;
} UP2P_HOST;

// 以下为应用层定义

// 设置密钥数据包
typedef struct{
	u32 key0;
	u32 key1;
} UP2P_SET_KEY;

// 令牌数据包
typedef struct{
	u32 token;
} UP2P_TOKEN;

// 配置服务器信息
typedef struct{
	u16 port;		// 服务器端口号
	char name[64];	// 服务器IP或者域名字符串
} UP2P_CONFIG_HOST;

//gpio 控制模式
enum{
	//GPIO_=0,		//普通gpio 输入输出
	GPIO_OUTPUT=0,
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


//外部函数
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
