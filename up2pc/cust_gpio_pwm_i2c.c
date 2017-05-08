
#ifndef _WIN32

#include "cust_config.h"
#include "iot_api.h"
#include "types.h"
#else
#include "../up2p.h"
#include "cust_config.h"



#endif

#define SHIFTLEFTBYTE(date,n)	(date << n)
#define RIGHTSHIFTBYTE(date,n)	(date >> n)
#define GETGPIOCNST(pin)		(gpios_cnst[pin] & 0x0f)//获取pin的工作模式
#define GETGPIOINF(pin)		    (RIGHTSHIFTBYTE(gpios_cnst[pin],4))//获取pin的pwm结构索引

#define I2CADDRESS(addr)		SHIFTLEFTBYTE(addr,1)

#define ISASGPIOMODE(pin)		(GETGPIOCNST(pin) == GPIO_)
//#define ISASGPIOFREE(pin)		(GETGPIOCNST(pin) <= GPIO_INTRTRI)
#define ISASGPIOFREE(pin)		(pin == pin)

#define ISASPWMMODE(pin)   		(GETGPIOCNST(pin) == GPIO_PWM)
#define ISASI2C_SCKMODE(pin)	(GETGPIOCNST(pin) == GPIO_I2C_SCL)
#define ISASI2C_SDAMODE(pin)	(GETGPIOCNST(pin) == GPIO_I2C_SDA) 





/*gpios_cnst  :gpio 控制状态0~5 对应gpio  0~5
**|0~~~~~3|******|||4~~~~~~~~~~~~~~7|
**|gpio工作模式||其结构的地址   |
*/
static u8  gpios_cnst[GPIOPINS];	
static pwm_cust_	pwm_cust[PWMNBS];	
static u8 pwm_cust_cnt =0;	//当前有多少个gpio用作pwm
static I2C_cust_ i2c_cust;

#ifndef _WIN32
static TIMER_T	pwmsoftime;
#endif

static I2C_cust_ I2C_cust;
static u32 gpioread_tem;

void SoftI2c_init(void);

/*
*配置 gpio
*/
int gpio_init(u8 pin,u8 mode,u8 value)
{
	if( mode == eGPIO_Input)
		readgpio(pin,&value);
		else 
			writegpio(pin,value);
		return 0;
}
#ifdef _WIN32
int returngpiomode(u8 pin)
{
	return (GETGPIOCNST(pin)-1);
}
#endif
/*函数功能ｅ：配置gpio 为输入 并读取其值
*输入: pin  :gpio口;value(0 为低，否则为高)
*输出: -1 ( 设置失败，gpio已另作他用) ； 0 成功
*/
int readgpio(u8 pin,u8 *value)
{

 	u32 val_out;
	if( !ISASGPIOFREE(pin) | pin > GPIOPINS)
		return -1;
#ifndef _WIN32
	//设置成功则注册
	if(IoT_gpio_input(pin,&val_out) == 0)
		{
		gpios_cnst[pin] = GPIO_INPUT;
		value[0] = (val_out == 0)?0:1;
		return 0;
	}else 
		return -1;

#else 
	gpios_cnst[pin] = GPIO_INPUT;
	*value =1;
	return 0;
#endif
	
}
/*函数功能ｅ：配置gpio 为输出 
*输入: pin  :gpio口;value(0 为低，否则为高)
*输出: -1 ( 设置失败，gpio已另作他用) ； 0 成功
*/
int writegpio(u8 pin,u8 value)
{
	if( !ISASGPIOFREE(pin) | pin > GPIOPINS)
		return -1;
	//设置成功则注册
#ifndef _WIN32
	if(IoT_gpio_output(pin,value) == 0)
		{
		gpios_cnst[pin] = GPIO_INPUT;
		return 0;
	}else 
		return -1;

#else 
		gpios_cnst[pin] = GPIO_INPUT;
		return 0;
#endif
}
//产生pwm 函数
//根据freq 控制gpio的高低
#ifndef _WIN32
static void pwmtimer_fnc(u32 pere1, u32 pere2) 
{
	u8 i;
	u8 ppwm;
	//Printf_High(">>pwmtimer_fnc\n");
	for(i=0;i<GPIOPINS;i++)
		{
		if(!ISASPWMMODE(i))
			continue;
		//获取pwm 结构的索引
		ppwm = GETGPIOINF(i);
		if(ppwm > PWMNBS)
			{
			//Printf_High("pwm stc error : ppwm :%d\n",ppwm);
			return ;
		}
		//Printf_High("i :%d;ppwm :%d;pwm_cust[i].timer_cnt :%d;\n",i,ppwm,pwm_cust[ppwm].timer_cnt);
		++pwm_cust[ppwm].timer_cnt; 
		if(pwm_cust[ppwm].timer_cnt == pwm_cust[ppwm].duty)
			{

			IoT_gpio_output(i,PWM_HIGHT);
			}
			else if(pwm_cust[ppwm].timer_cnt == pwm_cust[ppwm].freq)
				{
				IoT_gpio_output(i,PWM_LOW);
				pwm_cust[ppwm].timer_cnt = 0;
			}
	}
	
	cnmTimerStartTimer(&pwmsoftime,PWMTIM_INV);//开启pwm定时器	
}
#endif
// 初始化pwm结构
void gpio_pwm__i2c_init(void)
{
	u8 i;
	SoftI2c_init();
	for( i=0;i<PWMNBS;i++)
		{
 //		pwm_cust[i].pin 	= 0;
		pwm_cust[i].freq	= 0 ;
		pwm_cust[i].duty	= 0 ;
	}
#ifndef _WIN32
	cnmTimerInitTimer(&pwmsoftime,pwmtimer_fnc,0,0);
#endif
}
/***
函数作用: 设置pwm  
输入:pin  作为pwm输出的gpio；freq :频率  ;duty : 占空比
输出0为成功，否则返回当前gpio的当前的工作模式
***/
int setpwm(u8 pin,u16 freq,u16 duty)
{

	int ret =-1;
	int i;
	Printf_High("setpwm pin :%2X; feq:%2X;duty:%2X \n",pin,freq,duty);
		

	if( !ISASGPIOFREE(pin) )
		return -1; 
	//注册pwm
	for(i=0;i<PWMNBS;i++)
		{
		if( pwm_cust[i].freq == 0  )
			break;
	}
	//pwm已经全部注册
	if(i == PWMNBS)
		return -1;
	//注册pin
	Printf_High("pwm pin : %d pwn_stc[ %d]\n",pin,i);
	gpios_cnst[pin] =  SHIFTLEFTBYTE(i,4)|GPIO_PWM;
	Printf_High("gpios_cnst[pin] : %2X\n",gpios_cnst[pin]);
	
	//组成pwm
	pwm_cust[i].freq = freq;
	pwm_cust[i].duty = duty;
#ifndef _WIN32
	if(pwm_cust_cnt == 0)
		cnmTimerStartTimer(&pwmsoftime,PWMTIM_INV);//开启pwm定时器
#endif
	pwm_cust_cnt++;
	
	return 0;
}
//改变频率
int changepwmfreqduty(u8 pin,u16 freq,u16 duty)
{
	u8 ppwm;
	Printf_High("changepwmfreqduty pin :%2X; feq:%2X;duty:%2X \n",pin,freq,duty);
	
	if(!ISASPWMMODE(pin))
		return -1;
	
	if(freq > MAXPWMFREQ)
		return -1;
	ppwm = GETGPIOINF(pin);  
	if( ppwm >= PWMNBS )
		{
		Printf_High("error :pwm struct[%d]\n",ppwm);
		return -1;
		}
	
	pwm_cust[ppwm].freq= freq;
	pwm_cust[ppwm].duty = duty;
	return 0;
}

//读取pwm的频率 占空比 
int readpinpwmfreqduty(u8 pin,u16 *freq,u16 *duty)
{
	u8 i;
	u8 ppwm;
	
	Printf_High("readpinpwmfreqduty pin :%2X;  \n",pin);
	ppwm = GETGPIOINF(pin);
	
	if(!ISASPWMMODE(pin))
		return -1;
	
	if( ppwm >= PWMNBS )
		{
		Printf_High("error :pwm struct[%d]\n",ppwm);
		return -1;
		}
	
	Printf_High("info  :%2X;  \n", ppwm);
	*freq = pwm_cust[ppwm].freq;
	*duty = pwm_cust[ppwm].duty;
	return 0;
}

/***
函数作用:  pin 设置为普通gpio 状态 不再作为pwm使用
输入: 
输出: 
修改历史:
***/
static int  releasepwmpin(u8 pin)	
{
	if(!ISASPWMMODE(pin))
		return -1;
	gpios_cnst[pin]	= 0;
	pwm_cust[GETGPIOINF(pin)].freq	= 0;
	pwm_cust[GETGPIOINF(pin)].duty	= 0;
#ifndef _WIN32
	if(--pwm_cust_cnt == 0)
		cnmTimerStopTimer(&pwmsoftime);
#endif
	return 0;
}
#ifdef I2C_SUP_

#define I2CSDAPIN	(i2c_cust.sda_pin)
#define I2CSCLPIN	(i2c_cust.clk_pin)
#ifndef _WIN32
//#define SDAOUT  	IoT_gpio_output((u32)I2CSDAPIN,I2C_OUT_HIGHT)// 配置SDA为输出端口 默认输出为高
//#define SDAIN   	IoT_gpio_input((u32)I2CSDAPIN,&gpioread_tem)// 配置SDA为输入端口默认管脚为低

#define SDAHIG  	IoT_gpio_output((u32)I2CSDAPIN,I2C_OUT_HIGHT) // sda 置高
#define SDALOW  	IoT_gpio_output((u32)I2CSDAPIN,I2C_OUT_LOW)  // sda 置低

#define SCLHIG  	IoT_gpio_output((u32)I2CSCLPIN,I2C_OUT_HIGHT) // SCL 置高
#define SCLLOW  	IoT_gpio_output((u32)I2CSCLPIN,I2C_OUT_LOW)  // SCL 置低

#define SDAREAD    	readgpio_sda()

#define Delay_US( n)		usecDelay(n)
#else 

#define SDAHIG  	 {}
#define SDALOW  	 {}

#define SCLHIG  	 {}
#define SCLLOW  	 {}

#define SDAREAD    	1

#define Delay_US( n)
#endif
//初始化 i2c
void SoftI2c_init(void)
{
	i2c_cust.clk_pin = 0;
	i2c_cust.sda_pin = 0;
	i2c_cust.speed	 = 0;
	i2c_cust.address = 0;	
	
}
//配置i2c 
int SoftI2c_Setup(u8 scl,u8 sda,u16 speek)
{
	Printf_High("scl :%2X;sda: %2X ; speek : %4X\n",scl,sda,speek);

#ifdef _WIN32
	return 0;
#endif

	if( (!ISASGPIOFREE(scl)) || (!ISASGPIOFREE(sda))||(speek > 400) )
		return -1;

	//目前有i2c 在使用
	if(i2c_cust.speed != 0 )
			return -2;
	//写入结构体
	i2c_cust.clk_pin = scl;
	i2c_cust.sda_pin = sda;
	i2c_cust.speed	 = speek;
	//注册
	gpios_cnst[scl]	= GPIO_I2C_SCL;
	gpios_cnst[sda] = GPIO_I2C_SDA;
	return 0;
}
//释放i2c 
int releaseSoftI2c(void)
{
	if(i2c_cust.speed ==0)
		return -1;

	i2c_cust.clk_pin = 0;
	i2c_cust.sda_pin = 0;
	i2c_cust.speed	 = 0;
	i2c_cust.address = 0;	
	gpios_cnst[i2c_cust.clk_pin] = 0;
	gpios_cnst[i2c_cust.sda_pin] = 0;

	return 0;
}
//读取sda gpio上的值
static int readgpio_sda(void)
{
	u32 value =0;	
	// 读SDA 值
#ifndef _WIN32
	if( IoT_gpio_input((u32)I2CSDAPIN,&value) <0)
		{
		Printf_High("gpio read error\n");
		return -1;
		}
#endif 

	return (value & 0x01);
}
//产生IIC起始信号
static void MyI2C_GenStart(void)
{

	//SDAOUT;     //sda线输出
	SDAHIG;     // sda 置高
	SCLHIG;     // SCL 置高
	Delay_US(4);
	SDALOW;     // sda 置低
	Delay_US(4);
	SCLLOW;     // SCL 置低
}	  
//产生IIC停止信号
static void MyI2C_GenStop(void)
{


	//SDAOUT;     //sda线输出

	SCLLOW;     // SCL 置低
	SDALOW;     // sda 置低
	Delay_US(4);
	SCLHIG;     // SCL 置高
	SDAHIG;     // sda 置高
	Delay_US(4);
}
//产生ACK应答
static void MyI2C_GenAck(void)
{


	SCLLOW;     // SCL 置低
	//SDAOUT;     //sda线输出
	SDALOW;     // sda 置低
	Delay_US(2);
	SCLHIG;     // SCL 置高
	Delay_US(2);
	SCLLOW;     // SCL 置低
}
//不产生ACK应答		    
static void MyI2C_GenNAck(void)  
{


	SCLLOW;     // SCL 置低
	//SDAOUT;     //sda线输出
	SDAHIG;     // sda 置高
	Delay_US(2);
	SCLHIG;     // SCL 置高
	Delay_US(2);
	SCLLOW;     // SCL 置低
	
}					 				     
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
static u8 MyI2C_WaitAck(void)
{
	u8 ucErrTime=0;


   //SDAIN;   // 配置SDA为输入端口

	SDAHIG;     Delay_US(2);// sda 置高
	SCLHIG;     Delay_US(2);// SCL 置高
	

	while(SDAREAD)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
            //SDAOUT;     //sda线输出
            SCLLOW;
			return 1;
		}
	}
	SCLLOW;     // SCL 置低

	return 0;  
} 

//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
static u8 MyI2C_SendByte(u8 txd)
{                        
    u8 t;   
	//SDAOUT;     //sda线输出
	SCLLOW;     // SCL 置低
    for(t=0;t<8;t++)
    {              
        if (txd&0x80)
        {
            SDAHIG;     // sda 置高
        }
        else
        {
            SDALOW;     // sda 置低
        }
        txd <<= 1; 	  

        Delay_US(2);
        SCLHIG;     // SCL 置高
        Delay_US(2);
        SCLLOW;     // SCL 置低
        Delay_US(2);
    }	

    return 0;
} 	    

//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
static u8 MyI2C_ReadByte(unsigned char ack)
{
	unsigned char i,receive=0;
	if(i2c_cust.speed ==0)
		return 0xff;

	//SDAIN;   // 配置SDA为输入端口
    for(i=0;i<8;i++ )
	{
        SCLLOW;     // SCL 置低
        Delay_US(2);
        SCLHIG;     // SCL 置高
        receive <<= 1;
        if(SDAREAD)
        {
            receive++;   
        }
        Delay_US(2);
    }	
    
    if (!ack)
    {
        MyI2C_GenNAck();//发送nACK
    }
    else
    {
        MyI2C_GenAck(); //发送ACK   
    }
    
    return receive;
}
volatile static u8 MyI2C_LockFlag = TRUE;
static u8 MyI2C_GetLockFlag(void)
{

    if (MyI2C_LockFlag==FALSE) return FALSE;
    MyI2C_LockFlag = FALSE;
    return TRUE;
}
static void MyI2C_ResLockFlag(void)
{
    MyI2C_LockFlag = TRUE;
}
//开始读取设备
// addres i2c 设备地址
int starreadI2c(u8 addres)
{
	//SDAIN;   // 配置SDA为输入端口
    MyI2C_GenStart();
	//设备 地址
    MyI2C_SendByte(I2CADDRESS(addres)|0x01);    
    
    if (MyI2C_WaitAck())        //Wait for Slave ACK
    { 
    	Printf_High(" i2c dev_ack error\n");
        MyI2C_GenStop();
        MyI2C_ResLockFlag();  // 释放
        return -1; 
    }
	return 0;

}
//读i2c数据
//address  : 数据源地址
//readbuf  : 读到的数据缓存
//len : 数据长度  byte
int I2c_Read(u8 addres,u8 *wbuf,u8 wlen,u8 *readBuf,u8 rlen)
{
	u8 i,j;

#ifdef _WIN32
	memset(readBuf, 'A', rlen);
	return rlen;
#endif

	if(i2c_cust.speed ==0)
			return -1;
    if (MyI2C_GetLockFlag() == FALSE)  
			return -1;  // 获取lock
	//检测scl  sda	pin 是否合法
	if(I2CSCLPIN >= GPIOPINS ||I2CSDAPIN>= GPIOPINS )
			return -2;	

	Printf_High("addres=%2X;wbuf=%2X;wlen=%2X;rlen =%2X\n",addres,wbuf[0],wlen,rlen);	
	Printf_High("i2c read:: sck_pin=%2X;sda pin =%2X\n",I2CSCLPIN,I2CSDAPIN);
	if(I2CSCLPIN >= GPIOPINS ||I2CSDAPIN>= GPIOPINS )
		return -2;

    if( starreadI2c(addres)< 0)
		return -1;
		
	if( wlen > 0) 
		{
		
		MyI2C_SendByte(wbuf[0]);    
	    if (MyI2C_WaitAck())        //Wait for Slave ACK
	    { 
	    	Printf_High(" i2c address_ack error\n");
	        MyI2C_GenStop();
	        MyI2C_ResLockFlag();  // 释放
	        return -1; 
	    } 
		if(wlen == 1)
			{
				if( starreadI2c(addres)< 0)
						return -1;
			}
	}

	for(i=0; i<(rlen - 1); i++)
		{
		readBuf[i] = MyI2C_ReadByte(1);  // read 第1 个并 发 ack
	}
	readBuf[i] = MyI2C_ReadByte(0);  // read 第len  个并 发 nack

	Printf_High("I2C len =%d; readbuff::: ",i);
	for(j=0;j>rlen;j++)
		Printf_High("%2X",readBuf[j]);
	//Printf_High(":::::\n************\n");

    MyI2C_GenStop(); 
    MyI2C_ResLockFlag();  // 释放
	return i;
}
//fnc:	写i2c 数据
//input:	addr  i2c 设备地址; wbuf  数据缓存 ； wlen 数据长度
//out :	-1 非法输入速度 或者 i2c 目前不可用；否则返回发送的字节数
int I2c_write(u8 addr,u8 *wbuf,u8 wlen)
{    
  		u8 i;

#ifdef _WIN32
		printf("wbuf = %s\n", wbuf);
		return wlen;
#endif
		if(i2c_cust.speed ==0)
			return -1;
        if (MyI2C_GetLockFlag() == FALSE)  
				return -1;  // 获取lock
		//检测scl  sda  pin 是否合法
		if(I2CSCLPIN >= GPIOPINS ||I2CSDAPIN>= GPIOPINS )
			return -2;	
		Printf_High("read:: dev_addres=%2X;data_address=%2X;len=%2X\n",addr,wbuf[0],wlen);

        MyI2C_GenStart();

        MyI2C_SendByte(I2CADDRESS(addr)|0x00);     
        
        if (MyI2C_WaitAck())        //Wait for Slave ACK
        {  
        	Printf_High(" i2c write dev ack error\n");
			MyI2C_GenStop();
            MyI2C_ResLockFlag();  // 释放
            return -1; 
        } 
		for( i=0; i< wlen ;i++)
			{
			MyI2C_SendByte(wbuf[i]);
			  
	        if (MyI2C_WaitAck())        //Wait for Slave ACK
	        {  
	        	Printf_High(" i2c writting ack error\n");
				MyI2C_GenStop();
	            MyI2C_ResLockFlag();  // 释放
	            return -1; 
	        } 
		}
		//Printf_High("writted len =%2X\n",i);
        MyI2C_GenStop();  
        MyI2C_ResLockFlag();  // 释放
   		return i;
}


#endif


#ifndef  _WIN32

typedef struct {  //  //一个结构体对应一个gpio
	INT8  gpin;
	INT8  ledsta_cf;		//控制等的状态
	INT32   gpioficktime;    //亮灯的时间
} gpioled;
static gpioled mt7681gpio[GPIOPINS];


/*****************************************
*functions: 根据 ledsta_cf 状态控gpio 的高低
* input :  pin  第几个gpio    matix :LEDon 的时间
***********************/
void changegpio_lv( INT32 pin,INT32  maxtime)
{
	if( pin >= GPIOPINS)
		return ;

	if( mt7681gpio[pin].ledsta_cf == LEDON){
		if( (mt7681gpio[pin].gpioficktime++) < maxtime){
			IoT_gpio_output(pin,LEDON);
			return ;
			}
			else if(mt7681gpio[pin].gpioficktime > (2*maxtime) ){
					mt7681gpio[pin].ledsta_cf 		= LEDOFF;
					mt7681gpio[pin].gpioficktime 	= 0;
					IoT_gpio_output(pin,LEDOFF);
					}
		}
			return;
	
	    
}
/*****************************************
*functions:  使能一个gpio 控制符
* input :  pin  第几个gpio     需要ledon
***********************/ 
void cf_gpioledon(INT32 pin){
	mt7681gpio[pin].ledsta_cf = LEDON;
	mt7681gpio[pin].gpioficktime	= 0;
	IoT_gpio_output(pin,LEDON);
}
/*****************************************
*functions:   所有的gpio  为low
* input :   
***********************/
void gpioalllow(void){
	INT8  i;
	for(i=0; i<GPIOPINS;i++){
		IoT_gpio_output(i,LEDOFF);
	}
	return ;
}

#endif