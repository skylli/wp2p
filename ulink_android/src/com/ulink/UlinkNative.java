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
 
	static public int	CMD_SEND_SERIAL			= 0x3000;	// ���ʹ�������
	static public int	CMD_SEND_SERIAL_ACK		= 0x3001;	// ���ʹ������ݻ�Ӧ
	static public int	CMD_READ_SERIAL			= 0x3010;	// ��ȡ��������
	static public int	CMD_READ_SERIAL_ACK		= 0x3011;	// ��ȡ�������ݻ�Ӧ
	static public int	CMD_TXRX_SERIAL			= 0x3020;	// ���շ�һ�����ʽ���ʹ�������
	static public int	CMD_TXRX_SERIAL_ACK		= 0x3021;	// ���շ�һ�����ʽ��Ӧ��������

	static public int 	CMD_WIFISDKVER_QURY		= 0x3030;	// ѯ��wifi sdk �İ汾��Ϣ
	static public int 	CMD_WIFISDKVER_QURY_ACK	= 0x3031;	// ѯ��wifi sdk �İ汾��Ϣ��Ӧ
	static public int	CMD_WIFIUPDATE_TRI		= 0x3040;	// ��������wifiSDK����
	static public int	CMD_WIFIUPDATE_TRI_ACK	= 0x3041;	// ��Ӧ����
	static public int	CMD_WIFIUPDATE_DATA		= 0x3050;	// ����wifi��������
	static public int	CMD_WIFIUPDATE_DATA_ACK	= 0x3051;	// ����wifi�������ݻ�Ӧ

	static public int	CMD_GPIO_INIT			= 0x3060;	// ����gpio
	static public int	CMD_GPIO_INIT_ACK		= 0x3061;	// ����gpio ��Ӧ
	static public int	CMD_GPIO_READ			= 0x3070;	// ��gpio
	static public int	CMD_GPIO_READ_ACK		= 0x3071;	// ��gpio ��Ӧ
	static public int	CMD_GPIO_WRITE			= 0x3080;	// дgpio
	static public int	CMD_GPIO_WRITE_ACK		= 0x3081;	// дgpio ��Ӧ
	static public int	CMD_PWM_INIT			= 0x3090;	// ����pwm
	static public int	CMD_PWM_INIT_ACK		= 0x3091;	// ����pwm ��Ӧ
	static public int	CMD_PWM_READ			= 0x30A0;	// ��pwm����
	static public int	CMD_PWM_READ_ACK		= 0x30A1;	// ��pwm������Ӧ
	static public int	CMD_PWM_WRITE			= 0x30B0;	// дpwm ����
	static public int	CMD_PWM_WRITE_ACK		= 0x30B1;	// дpwm ������Ӧ
	static public int	CMD_I2C_INIT			= 0x30C0;	// ����I2C
	static public int	CMD_I2C_INIT_ACK		= 0x30C1;	// I2C���û�Ӧ
	static public int	CMD_I2C_READ			= 0x30D0;	// ��ȡi2c����
	static public int	CMD_I2C_READ_ACK		= 0x30D1;	// ��ȡ��Ӧ
	static public int	CMD_I2C_WRITE			= 0x30E0;	// дi2c����
	static public int	CMD_I2C_WRITE_ACK		= 0x30E1;	// дi2c���ݻ�Ӧ
	static public int	CMD_I2C_TXRX			= 0x30F0;	// дi2c�ȴ�����
	static public int	CMD_I2C_TXRX_ACK		= 0x30F1;	// д��i2c���ݻ�Ӧ
	
	static public int  LOW_GPIO_LEV=0;      ///GPIO������
	static public int  HIGHT_GPIO_LEV=1;    ///GPIO������
	
	static public int eGPIO_Output=0; //  ���ģʽ
	static public int eGPIO_Input=1;  //  ����ģʽ

}

