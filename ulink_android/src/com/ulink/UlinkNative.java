package com.ulink;

public class UlinkNative {
		static {
	        System.loadLibrary("ulink");
	    }
	static public native int ulinkInit(String host, String appId);
	static public native int ulinkOpen(String devId,String key);
	static public native int ulinkConfig(String devId, String ssid,String passwd,int type,String keystr);
	static public native int ulinkSendOnline(int link);
	static public native int ulinkCheckOnline(int link);
	
	static public native int ulinkSendGPIOCmd(int link, int cmd, UlinkGPIO gpio);
	static public native int ulinkSendPWMCmd(int link, int cmd, UlinkPWM pwm);
	static public native int ulinkSendI2CCmd(int link, int cmd, UlinkIIC iic);
	static public native int ulinkSendCmd(int link, int cmd, String param, int len);
	
	static public native int ulinkWaitGPIOCmd(int link, int cmd, UlinkGPIO gpio);
	static public native int ulinkWaitPWMCmd(int link, int cmd, UlinkPWM pwm);
	static public native int ulinkWaitI2CCmd(int link, int cmd, UlinkIIC iic);
	static public native int ulinkWaitCmd(int link, int cmd, byte[] param, int maxlen);
	
	static public native int ulinkUpdateWifiUdpSend(int link,byte[] param, int len);
	static public native void ulinkBorderBegin();
	static public native void ulinkBorderEnd();
 
	static public int	CMD_SEND_SERIAL			= 0x3000;	// 发送串口数据
	static public int	CMD_SEND_SERIAL_ACK		= 0x3001;	// 发送串口数据回应
	static public int	CMD_READ_SERIAL			= 0x3010;	// 读取串口数据
	static public int	CMD_READ_SERIAL_ACK		= 0x3011;	// 读取串口数据回应
	static public int	CMD_TXRX_SERIAL			= 0x3020;	// 以收发一体的形式发送串口数据
	static public int	CMD_TXRX_SERIAL_ACK		= 0x3021;	// 返收发一体的形式回应串口数据

	static public int 	CMD_WIFISDKVER_QURY		= 0x3030;	// 询问wifi sdk 的版本信息
	static public int 	CMD_WIFISDKVER_QURY_ACK	= 0x3031;	// 询问wifi sdk 的版本信息回应
	static public int	CMD_WIFIUPDATE_TRI		= 0x3040;	// 触发更新wifiSDK命令
	static public int	CMD_WIFIUPDATE_TRI_ACK	= 0x3041;	// 回应更新
	static public int	CMD_WIFIUPDATE_DATA		= 0x3050;	// 传输wifi更新数据
	static public int	CMD_WIFIUPDATE_DATA_ACK	= 0x3051;	// 传输wifi更新数据回应

	static public int	CMD_GPIO_INIT			= 0x3060;	// 配置gpio
	static public int	CMD_GPIO_INIT_ACK		= 0x3061;	// 配置gpio 回应
	static public int	CMD_GPIO_READ			= 0x3070;	// 读gpio
	static public int	CMD_GPIO_READ_ACK		= 0x3071;	// 读gpio 回应
	static public int	CMD_GPIO_WRITE			= 0x3080;	// 写gpio
	static public int	CMD_GPIO_WRITE_ACK		= 0x3081;	// 写gpio 回应
	static public int	CMD_PWM_INIT			= 0x3090;	// 配置pwm
	static public int	CMD_PWM_INIT_ACK		= 0x3091;	// 配置pwm 回应
	static public int	CMD_PWM_READ			= 0x30A0;	// 读pwm参数
	static public int	CMD_PWM_READ_ACK		= 0x30A1;	// 读pwm参数回应
	static public int	CMD_PWM_WRITE			= 0x30B0;	// 写pwm 参数
	static public int	CMD_PWM_WRITE_ACK		= 0x30B1;	// 写pwm 参数回应
	static public int	CMD_I2C_INIT			= 0x30C0;	// 配置I2C
	static public int	CMD_I2C_INIT_ACK		= 0x30C1;	// I2C配置回应
	static public int	CMD_I2C_READ			= 0x30D0;	// 读取i2c数据
	static public int	CMD_I2C_READ_ACK		= 0x30D1;	// 读取回应
	static public int	CMD_I2C_WRITE			= 0x30E0;	// 写i2c数据
	static public int	CMD_I2C_WRITE_ACK		= 0x30E1;	// 写i2c数据回应
	static public int	CMD_I2C_TXRX			= 0x30F0;	// 写i2c等待数据
	static public int	CMD_I2C_TXRX_ACK		= 0x30F1;	// 写读i2c数据回应
	
	static public int  LOW_GPIO_LEV=0;      ///GPIO脚拉低
	static public int  HIGHT_GPIO_LEV=1;    ///GPIO脚拉高
	
	static public int eGPIO_Output=0; //  输出模式
	static public int eGPIO_Input=1;  //  输入模式

}

