#ifndef  cust_config_h
#ifndef _WIN32
#include "types.h"
#include "cust_udpsock.h"

#endif
#include "cust_gpio_pwm_i2c.h"

#define cust_config_h
//�汾��Ϣ
#define VER_ESP_SDK    "V0.0.0.1 "  
/*******************************
******************************HQ version v0.1  20140714
***fuction:
			1 ʵ��͸�����������Ĺ��ܵĺ꿪�� 		
****/
#define DATACM_MTKSUP_DIS	1// ע��mtk �ṩ�� data com  
//#define DEBUG_INFO			1 // ʹ�ܸ���debug output ���
#define HUQVERSION1		"0001"
#define HUQ_MAGIC		"000100000000"
#define HQBROADCAST		"0000@"
#define TCMD_DEFAULT	"Default"
#define ACKUART			"OK"
#define REBOOTCMDD		"#Wm035273740 "

#define DNSVLINKS_NAME	"p2p.wilddog.cn"
#define ONLINETIM_INV	 25000  // ����ʱ����25s
#define DNSLLTIM_INV	300000  // ����ʱ����5M
#define DNSSSTIM_INV	120000  // ����ʱ����2M

#define READSERIALDEL_INV	200  //  ������������ӳ�50ms


//smnt timeout max
#define SMNTTIMEOUT_INV  500	//500MS
#define SMNTTIMEOUT_MX	 30     //5*500 = 2.5s


/**
*	udp port config
**/
#define LESTENINGPORT      27100
#define SERVESVLINKSPORT   27102   // wilddog ip ��������ַ p2p.wilddog.cn �˿� 7685

#define ONLINEPORT         37102

/**********************************************************************
gpio define  and strcut
**********************************************************************/
#define GPIOPINS    		6
#define	PWMNBS				5	//֧�ֵ�pwm
#define MAXPWMFREQ          1000
#define PWM_HIGHT			1
#define PWM_LOW				0
#define PWMTIM_INV			1 //10MS

#define POWERLED_P			0
#define NETCONNETLED_P    	1
#define NETPINNGLED_P		3
#define UARTMODE_P			3
#define UARTRXLED_P			4

#define LEDON				1
#define LEDOFF				0
#define NETPINNGLEDON   	15
#define UARTRCLEDONT   		15
/**********************************************************************
**	flash config  user flash address  star address 0x70
**********************************************************************/

#define FLASH_CUST_CFG_STARADDRESS		(0X70)
#define FLASH_USR_CFG_STARADDRESS		(FLASH_OFFSET_USR_CFG_START 	\
											+FLASH_CUST_CFG_STARADDRESS)	\
											 
#define FLASH_CUST_CFG_KEY				(0X00)
#define FLASH_CUST_CFG_KEY_LEN			(0X08)
#define KEY_FLASH_ADDRESS				(FLASH_USR_CFG_STARADDRESS+FLASH_CUST_CFG_KEY)
//wifi update flash address
#define MODEADDR_   	0x18001

//I2C  
#define I2C_SUP_  1
#define I2C_OUT_HIGHT	1
#define I2C_OUT_LOW	    0



	
//#define _UARTDEBUG     		1

//#define _UDPDEBUG
#ifdef _UARTDEBUG
#define UARTDEBUG(Fmt)    	\
{                                   \
                                \
       Printf_High  Fmt;					\
                                 \
} 
#else
#define UARTDEBUG(Level, Fmt)   {}

#endif	


#ifdef _UDPDEBUG
#define UDPDEBUG(Fmt)    	\
{                            \
                                \
       Printf_High  (Fmt);					\
                                 \
} 
#else
#define UDPDEBUG(Fmt)   {}

#endif	


/**********************************************************************
**		fuction  declare
*******************************************************************/
#ifndef _WIN32

void cf_gpioledon(INT32 pin);
void changegpio_lv( INT32 pin,INT32  maxtime);
void gpioalllow(void);

#endif

#endif
