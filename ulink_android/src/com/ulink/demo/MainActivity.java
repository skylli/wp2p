package com.ulink.demo;




import java.io.UnsupportedEncodingException;
import java.util.List;

import com.ulink.UlinkIIC;
import com.ulink.UlinkNative;
import com.ulink.UlinkGPIO;
import com.ulink.UlinkPWM;

import android.net.wifi.ScanResult;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;

import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.Toast;

public class MainActivity extends Activity implements OnClickListener {
    
	private byte AuthModeOpen = 0x00;
	private byte AuthModeShared = 0x01;
	private byte AuthModeAutoSwitch = 0x02;
	private byte AuthModeWPA = 0x03;
	private byte AuthModeWPAPSK = 0x04;
	private byte AuthModeWPANone = 0x05;
	private byte AuthModeWPA2 = 0x06;
	private byte AuthModeWPA2PSK = 0x07;   
	private byte AuthModeWPA1WPA2 = 0x08;
	private byte AuthModeWPA1PSKWPA2PSK = 0x09;
	
	static public final int VIEW_START_CONFIG=0;
	static public final int VIEW_CONFIGING=1;
	static public final int VIEW_LINK=2;
	static public final int VIEW_WRITE_RET=3;
	static public final int VIEW_READ_RET=4;
	static public final int VIEW_UNLINK=5;
	private Context mCot;
	private WifiManager mWifiManager;
	private String mConnectedSsid;
	private String mAuthString;
	private byte mAuthMode;
	Button gpio_light;
	Button gpio_write;
	Button gpio_read;
	Button iic_write;
	Button iic_read;
	Button pwm_write;
	Button pwm_read;
	Button btn_cfg;
	EditText edit_id;
	String passwd;
	String devId;
	ProgressBar pb_bar;
	ImageView iv_gpio0;
	String keyStr="1234567890000000";
	
	boolean gpio0Status_flag=false;
	boolean configOver=false;
	boolean daemonFlag=false;
	boolean cmdRunFlag=true;
	boolean daemonPause=false;
	char recvbuf[]=new char[1024];
	int link=0; ///保存连接地址

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mCot=MainActivity.this;
        
        gpio_light = (Button) findViewById(R.id.gpio_light);
        gpio_write = (Button) findViewById(R.id.gpio_write);
        gpio_read = (Button) findViewById(R.id.gpio_read);
        iic_write = (Button) findViewById(R.id.iic_write);
        iic_read = (Button) findViewById(R.id.iic_read);
        pwm_write = (Button) findViewById(R.id.pwm_write);
        pwm_read = (Button) findViewById(R.id.pwm_read);
        btn_cfg = (Button) findViewById(R.id.btn_cfg);
        edit_id = (EditText) findViewById(R.id.edit_id);
        iv_gpio0 = (ImageView) findViewById(R.id.iv_gpio0);
        pb_bar = (ProgressBar) findViewById(R.id.pb_cfg);
        pb_bar.setVisibility(View.GONE);
		
        gpio_light.setOnClickListener(this);
        gpio_write.setOnClickListener(this);
        gpio_read.setOnClickListener(this);
        iic_write.setOnClickListener(this);
        iic_read.setOnClickListener(this);
        pwm_write.setOnClickListener(this);
        pwm_read.setOnClickListener(this);
        btn_cfg.setOnClickListener(this);
        new Thread(new Runnable() {
			@Override
			public void run() {
		       WifiManager mWifiManager = (WifiManager) getSystemService (Context.WIFI_SERVICE); 
			   WifiInfo info = mWifiManager.getConnectionInfo();
			   String mac=info.getMacAddress();
			   String appId="0000"+mac.replace(":", "");
			   MLog.d("appid:"+appId);
		       UlinkNative.ulinkInit("121.40.87.129", appId);
		       
             }
       }).start();
      
    }
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) { 
		    case R.id.gpio_write:
		    case R.id.gpio_light:
		    	new Thread(new Runnable() {
					@Override
					public void run() {
		            	byte recvbuf[] = new byte[32];
		            	int ret;
				    	UlinkGPIO ulinkgpio = new UlinkGPIO(); 
				    	ulinkgpio.pin=0;
				    	ulinkgpio.mode=UlinkNative.eGPIO_Output;
				    	if(gpio0Status_flag)
				    	    ulinkgpio.value=UlinkNative.LOW_GPIO_LEV;
				    	else
				    		ulinkgpio.value=UlinkNative.HIGHT_GPIO_LEV;	
				    	
				    	cmdRunFlag=true;
				    	UlinkNative.ulinkBorderBegin();
				    	UlinkNative.ulinkSendGPIOCmd(link,UlinkNative.CMD_GPIO_INIT,ulinkgpio);
				    	ret=UlinkNative.ulinkWaitCmd(link, UlinkNative.CMD_GPIO_INIT_ACK, recvbuf, 32);
				    	UlinkNative.ulinkSendGPIOCmd(link,UlinkNative.CMD_GPIO_WRITE,ulinkgpio);
				    	ret=UlinkNative.ulinkWaitCmd(link, UlinkNative.CMD_GPIO_WRITE_ACK, recvbuf, 32);
				    	UlinkNative.ulinkBorderEnd();
				    	cmdRunFlag=false;
				    	  if(ret>=0)
				    	  {	  
				    		  gpio0Status_flag=!gpio0Status_flag;
				    		  sendMsgStr(VIEW_WRITE_RET,"写GPIO成功  pin:"+ulinkgpio.pin+" mode:"+ulinkgpio.mode+" value:"+ulinkgpio.value);

				    	  }else
				    	  {
				    		  sendMsgStr(VIEW_WRITE_RET,"写GPIO失败");

				    	  }
		                 }
		           }).start();
		    	break;
		    case R.id.gpio_read:
		    	new Thread(new Runnable() {
					@Override
					public void run() {
						byte recvbuf[] = new byte[32];
						int ret=0;
				    	UlinkGPIO ulinkgpio = new UlinkGPIO(); 
				    	ulinkgpio.pin=0;
				    	ulinkgpio.mode=UlinkNative.eGPIO_Output;
				    	UlinkGPIO reculinkgpio = new UlinkGPIO(); 
				    	cmdRunFlag=true;
				    	UlinkNative.ulinkBorderBegin();
				    	//UlinkNative.ulinkSendGPIOCmd(link,UlinkNative.CMD_GPIO_INIT,ulinkgpio);
				    	///int ret=UlinkNative.ulinkWaitCmd(link, UlinkNative.CMD_GPIO_INIT_ACK, recvbuf, 32);
   					    	
				    	UlinkNative.ulinkSendGPIOCmd(link,UlinkNative.CMD_GPIO_READ,ulinkgpio);
				    	 ret=UlinkNative.ulinkWaitGPIOCmd(link, UlinkNative.CMD_GPIO_READ_ACK, reculinkgpio);
				    	UlinkNative.ulinkBorderEnd();
				    	cmdRunFlag=false;
				    	  if(ret>=0)
				    	  {	  
				    		  sendMsgStr(VIEW_WRITE_RET,"读GPIO成功,pin:"+reculinkgpio.pin+" mode:"+reculinkgpio.mode+" value:"+reculinkgpio.value);

				    	  }else
				    	  {
				    		  sendMsgStr(VIEW_WRITE_RET,"读GPIO失败");

				    	  }
		                 }
		    	}).start();
		    	break;
		    case R.id.iic_write:
		    	new Thread(new Runnable() {
					@Override
					public void run() {
		            	byte recvbuf[] = new byte[32];
		            	int ret;
				    	UlinkIIC ulinki2c = new UlinkIIC();
				    	ulinki2c.clk_pin=0;
				    	ulinki2c.sda_pin=1;
				    	ulinki2c.speed=10;
				    	
				    	cmdRunFlag=true;
				    	UlinkNative.ulinkBorderBegin();
				    	UlinkNative.ulinkSendI2CCmd(link,UlinkNative.CMD_I2C_INIT,ulinki2c);
				    	ret=UlinkNative.ulinkWaitCmd(link, UlinkNative.CMD_I2C_INIT_ACK, recvbuf, 32);
				    	
				    	ulinki2c.address = 0x50;
				    	ulinki2c.speed=10;
				    	ulinki2c.r_len = 0;
				    	ulinki2c.w_len= 2;
				    	ulinki2c.date=new byte[32];
				    	ulinki2c.date[0] =(byte) 0x30;
				    	ulinki2c.date[1] =(byte) 0x32;
				    	ulinki2c.date[2] =(byte) 0x00;
	
				    	UlinkNative.ulinkSendI2CCmd(link,UlinkNative.CMD_I2C_WRITE,ulinki2c);
				    	ret=UlinkNative.ulinkWaitCmd(link, UlinkNative.CMD_I2C_WRITE_ACK, recvbuf, 32);
				    	UlinkNative.ulinkBorderEnd();
				    	cmdRunFlag=false;
				    	  if(ret>=0)
				    	  {	  
				    		  sendMsgStr(VIEW_WRITE_RET,"写I2C成功 ");

				    	  }else
				    	  {
				    		  sendMsgStr(VIEW_WRITE_RET,"写I2C失败");

				    	  }
		                 }
		           }).start();
		    	
		    	break;
		    case R.id.iic_read:
		    	new Thread(new Runnable() {
					@Override
					public void run() {
		            	byte recvbuf[] = new byte[32];
		            	int ret;
				    	UlinkIIC ulinki2c = new UlinkIIC();
				    	ulinki2c.clk_pin=0;
				    	ulinki2c.sda_pin=1;
				    	ulinki2c.speed=10;
				    	cmdRunFlag=true;
				    	UlinkNative.ulinkBorderBegin();
				    	UlinkNative.ulinkSendI2CCmd(link,UlinkNative.CMD_I2C_INIT,ulinki2c);
				    	ret=UlinkNative.ulinkWaitCmd(link, UlinkNative.CMD_I2C_INIT_ACK, recvbuf, 32);
				    	
				    	ulinki2c.address = 0x50;
				    	ulinki2c.r_len = 1;
				    	ulinki2c.w_len= 1;
				    	ulinki2c.date=new byte[32];
				    	ulinki2c.date[0] =(byte) 0xa1;
				    	
				    	UlinkNative.ulinkSendI2CCmd(link,UlinkNative.CMD_I2C_READ,ulinki2c);
				    	ret=UlinkNative.ulinkWaitCmd(link, UlinkNative.CMD_I2C_READ_ACK, recvbuf, 32);
				    	UlinkNative.ulinkBorderEnd();
				    	cmdRunFlag=false;
				    	  if(ret>=0)
				    	  {	  
				    		  String recStr=null;
							try {
								recStr = new String(recvbuf,"UTF-8");
							} catch (UnsupportedEncodingException e) {
								// TODO Auto-generated catch block
								e.printStackTrace();
							}
							sendMsgStr(VIEW_WRITE_RET,"读I2C成功: "+recStr);

				    	  }else
				    	  {
				    		  sendMsgStr(VIEW_WRITE_RET,"读I2C失败");

				    	  }
		                 }
		           }).start();
		    	break;
		    case R.id.pwm_write:
		    	new Thread(new Runnable() {
					@Override
					public void run() {
		            	byte recvbuf[] = new byte[32];
		            	int ret;
		            	UlinkPWM ulinkpwm = new UlinkPWM(); 
		            	ulinkpwm.pin=1;
		            	ulinkpwm.freq=9;
		            	ulinkpwm.duty=5;
		            	cmdRunFlag=true;
				    	UlinkNative.ulinkBorderBegin();
				    	UlinkNative.ulinkSendPWMCmd(link,UlinkNative.CMD_PWM_INIT,ulinkpwm);
				    	ret=UlinkNative.ulinkWaitCmd(link, UlinkNative.CMD_PWM_INIT_ACK, recvbuf, 32);
				       	ulinkpwm.pin=1;
		            	ulinkpwm.freq=500;
		            	ulinkpwm.duty=100;
				    	UlinkNative.ulinkSendPWMCmd(link,UlinkNative.CMD_PWM_WRITE,ulinkpwm);
				    	ret=UlinkNative.ulinkWaitCmd(link, UlinkNative.CMD_PWM_WRITE_ACK, recvbuf, 32);
				    	UlinkNative.ulinkBorderEnd();
				    	cmdRunFlag=false;
				    	  if(ret>=0)
				    	  {	  
				    		  sendMsgStr(VIEW_WRITE_RET,"写PWM成功  pin:"+ulinkpwm.pin+" freq:"+ulinkpwm.freq+" duty:"+ulinkpwm.duty);

				    	  }else
				    	  {
				    		  sendMsgStr(VIEW_WRITE_RET,"写PWM失败");

				    	  }
		                 }
		           }).start();
		    	break;
		    case R.id.pwm_read:
		    	new Thread(new Runnable() {
					@Override
					public void run() {
		            	byte recvbuf[] = new byte[32];
		            	int ret;
		            	UlinkPWM ulinkpwm = new UlinkPWM(); 
		            	//ulinkpwm.pin=1;
		            	//ulinkpwm.freq=9;
		            	//ulinkpwm.duty=5;

		            	UlinkPWM recvpwm = new UlinkPWM(); 
		            	cmdRunFlag=true;
				    	UlinkNative.ulinkBorderBegin();
				    	//UlinkNative.ulinkSendPWMCmd(link,UlinkNative.CMD_PWM_INIT,ulinkpwm);
				    	//ret=UlinkNative.ulinkWaitCmd(link, UlinkNative.CMD_PWM_INIT_ACK, recvbuf, 32);
				       	ulinkpwm.pin=1;

				    	UlinkNative.ulinkSendPWMCmd(link,UlinkNative.CMD_PWM_READ,ulinkpwm);
				    	ret=UlinkNative.ulinkWaitPWMCmd(link, UlinkNative.CMD_PWM_READ_ACK, recvpwm);
				    	UlinkNative.ulinkBorderEnd();
				    	cmdRunFlag=false;
				    	  if(ret>=0)
				    	  {	  
				    		  sendMsgStr(VIEW_WRITE_RET,"读PWM成功  pin:"+recvpwm.pin+" freq:"+recvpwm.freq+" duty:"+recvpwm.duty);

				    	  }else
				    	  {
				    		  sendMsgStr(VIEW_WRITE_RET,"读写PWM失败");

				    	  }
		                 }
		           }).start();
		    	break;
		    case R.id.btn_cfg:
		    	final String input=edit_id.getText().toString();
		    	
		    	if(input!=null&&input.length()==16)
		    	{    
		    		  devId=input;	
		    		  getWifiInfo();
		    		  if(mConnectedSsid==null)
				      {
				    		Toast.makeText(getApplicationContext(), "请先打开wifi网络!", Toast.LENGTH_SHORT).show();
				    		break;
				      }
		    		  final EditText editText=new EditText(mCot);
		    		  if(passwd!=null)
		    			  editText.setText(passwd);
		    		  new AlertDialog.Builder(mCot).setCancelable(true).setTitle("请输入当前wifi密码").setMessage(mConnectedSsid)
						.setView(editText).setPositiveButton("确定",new DialogInterface.OnClickListener()
						{
						 @Override
						 public void onClick(DialogInterface dialog,int which) {	
							    passwd= editText.getText().toString();	
							   btn_cfg.setClickable(false);
							   daemonPause=true;
								new Thread(new Runnable() {
									@Override
									public void run() {
									      boolean isOk=false;	
									      Message msg = new Message();
										  msg.what=VIEW_START_CONFIG;
										  mHandler.sendMessage(msg);	
	                                        int ret=0;
											if((ret=UlinkNative.ulinkConfig(devId, mConnectedSsid, passwd, mAuthMode, keyStr))==0)
											{

													int openRet=0;							
													MLog.d("keyStr:"+keyStr);
													openRet=UlinkNative.ulinkOpen(devId, keyStr);
													if(openRet!=0)
													 {		
														 link=openRet;
														 isOk=true;
													 }
													MLog.d("openRet:"+openRet);
											}else
											{
												     MLog.d("config fail:"+ret);
											}
										  daemonPause=false;
										  Message ms = new Message();
										  configOver=true;
										  ms.obj=isOk;
										  ms.what=VIEW_LINK;
										  mHandler.sendMessage(ms);	
									
									}
									}).start();	
					 }} ).show();	 	
		    	}else
		    	{
		    		Toast.makeText(getApplicationContext(), "请输入16位设备id!", Toast.LENGTH_SHORT).show();
		    	}
		    	break;
		   	default:
		    		break;
		}	
	}
	private void readGPIO(final int gpio)
	{
		new Thread(new Runnable() {
			@Override
			public void run() {
				byte recvbuf[] = new byte[32];
		    	UlinkGPIO ulinkgpio = new UlinkGPIO(); 
		    	ulinkgpio.pin=gpio;
		    	ulinkgpio.mode=UlinkNative.eGPIO_Input;
		    	UlinkGPIO reculinkgpio = new UlinkGPIO(); 
		    	cmdRunFlag=true;
		    	UlinkNative.ulinkBorderBegin();
		    	UlinkNative.ulinkSendGPIOCmd(link,UlinkNative.CMD_GPIO_INIT,ulinkgpio);
		    	int ret=UlinkNative.ulinkWaitCmd(link, UlinkNative.CMD_GPIO_INIT_ACK, recvbuf, 32);
				    	
		    	UlinkNative.ulinkSendGPIOCmd(link,UlinkNative.CMD_GPIO_READ,ulinkgpio);
		    	 ret=UlinkNative.ulinkWaitGPIOCmd(link, UlinkNative.CMD_GPIO_READ_ACK, reculinkgpio);
		    	UlinkNative.ulinkBorderEnd();
		    	cmdRunFlag=false;
		    	  if(ret>=0)
		    	  {	  
		    		  if(reculinkgpio.value==1)
		    		      gpio0Status_flag=true;
		    		  else
		    			  gpio0Status_flag=false;
		    		  sendMsgStr(VIEW_WRITE_RET,"读GPIO成功,pin:"+reculinkgpio.pin+" mode:"+reculinkgpio.mode+" value:"+reculinkgpio.value);

		    	  }else
		    	  {
		    		  sendMsgStr(VIEW_WRITE_RET,"读GPIO失败");

		    	  }
                 }
    	}).start();
	}
	private void getWifiInfo()
	{
		mWifiManager = (WifiManager) getSystemService (Context.WIFI_SERVICE); 
		if(mWifiManager.isWifiEnabled())
		{
        	WifiInfo WifiInfo = mWifiManager.getConnectionInfo();
        	mConnectedSsid = WifiInfo.getSSID();
        	WifiInfo.getIpAddress();
			int iLen = mConnectedSsid.length();
			if (mConnectedSsid.startsWith("\"") && mConnectedSsid.endsWith("\""))
			{
				mConnectedSsid = mConnectedSsid.substring(1, iLen - 1);
			}		
			List<ScanResult> ScanResultlist = mWifiManager.getScanResults();
			for (int i = 0, len = ScanResultlist.size(); i < len; i++) {
				ScanResult AccessPoint = ScanResultlist.get(i);				
				if (AccessPoint.SSID.equals(mConnectedSsid))
				{		
					boolean WpaPsk = AccessPoint.capabilities.contains("WPA-PSK");
		        	boolean Wpa2Psk = AccessPoint.capabilities.contains("WPA2-PSK");
					boolean Wpa = AccessPoint.capabilities.contains("WPA-EAP");
		        	boolean Wpa2 = AccessPoint.capabilities.contains("WPA2-EAP");			
					if (AccessPoint.capabilities.contains("WEP"))
					{
						mAuthString = "OPEN-WEP";
						mAuthMode = AuthModeOpen;
						break;
					}
					if (WpaPsk && Wpa2Psk)
					{
						mAuthString = "WPA-PSK WPA2-PSK";
						mAuthMode = AuthModeWPA1PSKWPA2PSK;
						break;
					}
					else if (Wpa2Psk)
					{
						mAuthString = "WPA2-PSK";
						mAuthMode = AuthModeWPA2PSK;
						break;
					}
					else if (WpaPsk)
					{
						mAuthString = "WPA-PSK";
						mAuthMode = AuthModeWPAPSK;
						break;
					}
					if (Wpa && Wpa2)
					{
						mAuthString = "WPA-EAP WPA2-EAP";
						mAuthMode = AuthModeWPA1WPA2;
						break;
					}
					else if (Wpa2)
					{
						mAuthString = "WPA2-EAP";
						mAuthMode = AuthModeWPA2;
						break;
					}
					else if (Wpa)
					{
						mAuthString = "WPA-EAP";
						mAuthMode = AuthModeWPA;
						break;
					}									
					mAuthString = "OPEN";
					mAuthMode = AuthModeOpen;		
				}
			}
		}	
	}
	private void sendMsgStr(int msgNum,String obj)
	{
	   Message msg = new Message();
	   msg.what = msgNum;
	   msg.obj=obj;
	   mHandler.sendMessage(msg);
	}
	private void sendMsgInt(int msgNum,int arg)
	{
	   Message msg = new Message();
	   msg.what = msgNum;
	   msg.arg1=arg;
	   mHandler.sendMessage(msg);
	}
    Handler mHandler = new Handler(){
		@Override
		public void handleMessage(Message msg) {
			super.handleMessage(msg);

			switch(msg.what)
			{
			 case VIEW_START_CONFIG:
				  pb_bar.setVisibility(View.VISIBLE);	
				  pb_bar.setMax(170);
			      pb_bar.setProgress(0);
			      configOver=false;
				  new Thread(new Runnable() {
						@Override
						public void run() {
							sendMsgInt(VIEW_CONFIGING,20);
							sleep(1000);
							sendMsgInt(VIEW_CONFIGING,40);
							int i=0;
							while(configOver==false)
							{  
								i++;
								if(i==130)
									break;
								sleep(1000);
								sendMsgInt(VIEW_CONFIGING,40+i);
							}
						}}).start();
				  break;
			 case VIEW_CONFIGING:
			      pb_bar.setProgress(msg.arg1);
				  break;
             case VIEW_LINK:
            	 pb_bar.setVisibility(View.GONE);
            	    btn_cfg.setClickable(true);
     			   final boolean isOk=(Boolean) msg.obj;
					String dtitle=null;
					String dmsg=null;
					if(isOk)
					{
						readGPIO(0);
						if(daemonFlag==false)
						{  
							daemonFlag=true;
						   checkOnlineDaemon();
						}
						dtitle="连接成功";
						dmsg="返回作操界面";
					}else
					{
						dtitle="连接失败";
						dmsg="请检查网络或重新连接";
					}		
					 new AlertDialog.Builder(mCot)
					.setTitle(dtitle)
					.setMessage(dmsg)
					.setCancelable(false)
					.setPositiveButton("确定",
							new DialogInterface.OnClickListener() {
								@Override
								public void onClick(DialogInterface dialog,int which) {				       
								}
							}).show();
					break;
             case VIEW_UNLINK:
            	 Toast.makeText(getApplicationContext(), "设备未连接！", Toast.LENGTH_SHORT).show();  
            	  break;
             case VIEW_WRITE_RET:
             case  VIEW_READ_RET:
            	   if(gpio0Status_flag)
            	   {
            		   gpio_light.setText("关");
            		   iv_gpio0.setBackgroundResource(R.drawable.light_on);
            	   }else
            	   {
            		   gpio_light.setText("开");
            		   iv_gpio0.setBackgroundResource(R.drawable.light_off);
            	   }
            	   Toast.makeText(getApplicationContext(), (CharSequence) msg.obj, Toast.LENGTH_SHORT).show();  	 
            	 break;
            default:
            		 break;		
			}
		}
    };
	private void checkOnlineDaemon() {
		
		new Thread(new Runnable() {
			@Override
			public void run() { 
				   int ret=0;
					while (daemonFlag)
					{
						if(daemonPause==false)
						{
							if(cmdRunFlag==false)
							 {	
							      UlinkNative.ulinkSendOnline(link);
							      MLog.d("check online");
							 }
							 if((ret=UlinkNative.ulinkCheckOnline(link))<=0)
							  {
								 sendMsgStr(VIEW_UNLINK,"123");
							  }
							 MLog.d("check online daemon:"+ret);
						}
						 sleep(3000);
						
					}
			}
		}).start();
	}
    private void sleep(int time)
    {
    	
    	try {
			Thread.sleep(time);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    }

    
}
