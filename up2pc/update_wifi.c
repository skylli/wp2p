/*******************************************************************************

    This file is part of the xmode transport
    Copyright wilddog.com
    All right reserved.

    File:    update_wifi.c

    No description

    TIME LIST:
    CREATE  lxs   2014-8-13 17:47:42

*******************************************************************************/
#include "../up2p.h"
#include <stdio.h>
#include <stdlib.h>
#include "flash_map.h"

#ifndef  _WIN32
#include "types.h"
#include "cust_config.h"
#include "eeprom.h"
#include "iot_api.h"
#include "uart_sw.h"
#else
FILE *fp;

#endif

#define DEBUG_XMOD(...)	{}

#ifdef  XMODE_SUP

#ifndef  _WIN32
#define MAXUPDATETIMOUT_CNT	120	//1m = 60 * 2 * 1 = 120     // 1分钟内没有更新包 则推出更新
#define UPDTIMOUT_INV		500//500MS
TIMER_T	      updatetimerout;      //wifi update reading  timerout  500ms peer
#else 
#define Printf_High   printf

#endif


typedef struct{
	u8 updateting_sta;	//当前是否处于更新状态
	u8 packetno;	//当前包序号
	u8 packetno_old;//上一包序号 
	u8 retrans ;	//重发计数器
	u8	type;
    u8 updaetimout_cnt; //等待更新包 计数器
	
	long flashwritepack_cnt;
 	int flash_error_count ;		//flash 错误
    int UpgSize  ;
 	int len;		//当前写入flash 的字节数

	u8  Xcycle_upd ;		  /*if packetno >255 ,  flash 翻页*/
	u32 pktNum;		  /*if packetno >255 , use this parameter*/

	u8 Hap_Lsta_fig;//当前为ap 区域   0  当前在sta 区域

} wifiupdate_stc;
static wifiupdate_stc update_wifi;

//u8 retrans = PACKET_RETRY_LIMIT;//重发计数器
//static 	int flash_error_count = 0;		//flash 错误
//static int    UpgSize  = FLASH_UPG_FW_SIZE;



int flashcopy_Update_FW(VOID);



//初始化wifi 更新的结构体
int updatewifi_int(void)
{
	update_wifi.len		= 0;
	update_wifi.type	= 0;
	update_wifi.packetno= 1;
	update_wifi.packetno_old =0;
	update_wifi.retrans	= PACKET_RETRY_LIMIT;
	update_wifi.UpgSize = FLASH_UPG_FW_SIZE;
	update_wifi.flash_error_count = 0;
    update_wifi.flashwritepack_cnt =0;
	update_wifi.updateting_sta	=0;	//开始更新

	update_wifi.Xcycle_upd =0;
	update_wifi.pktNum	   =0;
	update_wifi.updaetimout_cnt =0;
	/*
	spi_flash_read((UINT32)MODEADDR_, &update_wifi.Hap_Lsta_fig, 1); //判断当前使用的是那个区域的固件
	if(isrunning_APcode())
		Printf_High("runing ap_address_code\n");
	 else 
	 	Printf_High("rurnning sta_address code \n");
	 	*/
}
#ifndef  _WIN32
void updatetim_calback(u32 parm1,u32 parm2)
{
	if(++(update_wifi.updaetimout_cnt) > MAXUPDATETIMOUT_CNT )
		{
			Printf_High("update timeout \n");
			updatewifi_int();
			cnmTimerStopTimer(&updatetimerout);
	}
	else 
		cnmTimerStartTimer(&updatetimerout, UPDTIMOUT_INV);
}

//初始化 远程更新超时时钟
void updtim_int(void)
{ 
	
	Printf_High("updtim_int \n");
	cnmTimerInitTimer(&updatetimerout,updatetim_calback,0,0);
}
#endif

//超时计数器置零 收到一包固件更新数据
void updaetimout_cnt_int(void)
{
	update_wifi.updaetimout_cnt =0;
}
//准备更新
int updatewifistar_int(void)
{
	update_wifi.len		= 0;
	update_wifi.type	= 0;
	update_wifi.packetno= 1;
	update_wifi.packetno_old =0;
	update_wifi.retrans	= PACKET_RETRY_LIMIT;
	update_wifi.UpgSize = FLASH_UPG_FW_SIZE;
	update_wifi.flash_error_count = 0;
    update_wifi.flashwritepack_cnt =0;
	update_wifi.updateting_sta	=1;	//开始更新
	
	update_wifi.pktNum	   =0;
	update_wifi.Xcycle_upd =0;

#ifndef _WIN32
		cnmTimerStartTimer(&updatetimerout, UPDTIMOUT_INV);
#else 
		creatnewbinfile("copyfile.bin");
	
#endif

	//spi_flash_read((UINT32)MODEADDR_, &update_wifi.Hap_Lsta_fig, 1); //判断当前使用的是那个区域的固件
}
int isrunning_APcode(void)
{
	return (update_wifi.Hap_Lsta_fig);
}
//处于开始阶段
int  updatestar_wifi(void)
{
	return (update_wifi.updateting_sta < 2);
}
//处于传输更新数据阶段
int  updateing_wifi(void)
{
	return (update_wifi.updateting_sta > 0);
}
//设置更新标识
int setupdateing_wifi(void)
{
	update_wifi.updateting_sta =2;
	return  update_wifi.updateting_sta;
}
#ifdef _WIN32
// 建立一个文件
 int creatnewbinfile(char const *filename)
{
	int ret =-1;
	
	remove(filename);
    fp = fopen(filename,"ab");
	Printf_High("creatnewbinfile\n");
	if(fp ==NULL )
		{
		perror("new_mt7681.bin:");
		return -1;
	}
	//重置pack 序号
	//packetno = 1; //
	//retrans  = PACKET_RETRY_LIMIT;
	return 0;
}
#endif  //wind32
/**
***			xmode code
****/
/* 
This function calculates the CRC used by the "Modem Protocol" The first argument
is a pointer to the message block. The second argument is the number ofbytes
in the message block. The message block used by the Modem Protocol contains 
128 bytes. The function return value isan integer which contains the CRC.
The lo order 16 bits of this integer are the coefficients of the CRC. The 
The lo order bit is the lo order coefficient of the CRC. 
*/ 
int calcrc(char *ptr, int count)
{ 
	int crc = 0, i; 
	while(--count >= 0) { 
		crc = crc ^ (int)*ptr++ << 8; 
		for(i = 0; i < 8; ++i) 
			if(crc & 0x8000) crc = crc << 1 ^ 0x1021; 
				else 
					crc = crc << 1; 
	} 
	return (crc & 0xFFFF); 
}

static void flushinput(void)
{
	
	//while (port_inbyte(((DLY_1S)*3)>>1) >= 0) 
	;
}
//更新128B
//考虑到网络传输的丢包比较严重回应每一包的ack时应该填写上 下一包的序号和其补码
//
int Xmodem_Update_FW(u8 const *xbuff,u8 *txbuff)
{
	//unsigned char xbuff[150];  /*only support 128Byte xmodem*/
	int bufsz; //crcf  校验
	int buf_crc,crc_buf_add;
	int crc_cal;
	u8* pData	= NULL;

	static u8  UartUpgHeader[UART_FW_HEADER_TOTAL_SIZE] = UART_FW_HEADER;
	//int i;
#ifdef _WIN32
	u32 ret =-1;
	static long foffset =0;
#endif	
	unsigned char flash_write_result = 0;

   


	DEBUG_XMOD("Xmodem_Update_FW \n");
	Printf_High( "xbuff[0] :%2X,xbuff[1]:%2X,xbuff[2]:%2X;xbuff[3]:%2X;xbuff[131]:%2X;xbuff[132]:%2X ;xbuff[133]:%2X ;xbuff[134]:%2X  \n",\
 				xbuff[0],xbuff[1],xbuff[2],xbuff[3],xbuff[131],xbuff[132],xbuff[133],xbuff[134]);
	switch (xbuff[0])
 		{
 			case XMOD_SOH:
				//128传输
				updaetimout_cnt_int();
				DEBUG_XMOD("XMOD_SOH.\n");
 				bufsz = 128;
 				goto start_recv;
 			case XMOD_STX:
				
 				bufsz = 1024;
				updaetimout_cnt_int();
 				goto start_recv;
 			case XMOD_EOT:
 				//传输结束
				Printf_High("XMOD_EOT.\n");
 				flushinput();
				
 				txbuff[0] = XMOD_OK;
				txbuff[1] = XMOD_OK;		//下一包序号
				txbuff[2] = XMOD_OK;	//下一包 序号补码
				
 				if(update_wifi.flash_error_count == 0)
 				{							
#ifndef _WIN32
					cnmTimerStopTimer(&updatetimerout);
 					spi_flash_update_fw_done(update_wifi.type);
				//	Printf_High("update_wifi.type : %d\n",update_wifi.type);
#else 
						fclose(fp);
#endif				
 				}
 				else
					 goto xmod_CAN;
			   
 				return 3;
			case XMOD_OK:
				//原本是更新ap 则不需要复制
#ifndef _WIN32
					
				if(update_wifi.type != UART_FlASH_UPG_ID_AP_FW)
						flashcopy_Update_FW();
#endif
				txbuff[0] = XMOD_OK;
				txbuff[1] = XMOD_OK;		//下一包序号
				txbuff[2] = XMOD_OK;	//下一包 序号补码
				//终止下一包数据的到来
				updatewifi_int();	
				return 3;
				
 			case XMOD_CAN:
 				//终止传输
 				 goto xmod_CAN;
 				break;
 			default:
				//重发
				goto retry_xmod;
 				break;
 		}
 		
	

start_recv:
		Printf_High("start_recv\n");
		
		if(update_wifi.flash_error_count != 0){
			    Printf_High("flash_error_count error\n");
				goto xmod_CAN;
			}
		if(update_wifi.packetno_old == xbuff[1]) //收到上一包数据
			{
			if (--update_wifi.retrans <= 0)		//收到太多上一包数据也会终止
 					goto xmod_CAN;
				Printf_High("rec old pack..\n");
				txbuff[0] = XMOD_ACK;
				txbuff[1] = update_wifi.packetno;	//下一包序号
				txbuff[2] = ~update_wifi.packetno;	//下一包 序号补码
				return 3;
			}
	    crc_cal = calcrc(&xbuff[TXMODEBASE], TXBUFSIZE);
		crc_buf_add = TXBUFSIZE+TXMODEBASE;
		buf_crc  = xbuff[crc_buf_add+3]<<24|xbuff[crc_buf_add+2]<<16|xbuff[crc_buf_add+1]<<8|xbuff[crc_buf_add+0];
		Printf_High("crc_cal :%d; buf_crc:%d\n",crc_cal,buf_crc);
		
		Printf_High("packetno:%d,xbuff[1]:%d \n",update_wifi.packetno,xbuff[1]);
		//printf("packetno=%2X;xbuff[0] =%2X ;xbuff[1] =%2X ;xbuff[2] =%2X ;xbuff[131] =%2X ;xbuff[132] =%2X;xbuff[133] =%2X;\n",\
		//						packetno,xbuff[0],xbuff[1],xbuff[2],xbuff[131],xbuff[132],xbuff[133]);
		//判断包头 //crc 校验
		if (xbuff[1] == (unsigned char)(~xbuff[2]) &&\
			(xbuff[1] == update_wifi.packetno || xbuff[1] == (unsigned char)update_wifi.packetno-1) &&\
			(crc_cal == buf_crc ))
		{
			//判断包序号
			if (xbuff[1] == update_wifi.packetno)
			{
				int count = update_wifi.UpgSize - update_wifi.len;
				
				pData = &xbuff[3];
				
				if (count > bufsz)
					count = bufsz;
				//处理收到的数据
				if (count > 0)
			  	{
#ifndef _WIN32
					DEBUG_XMOD("star_update...1\n");
					/*if packetno > 255 , it will overflow, thus , 
					we must use pktNum as the flash write offset */
					if((update_wifi.packetno == 0) && (update_wifi.pktNum% 256 == 255))
					{
						update_wifi.Xcycle_upd++;
					}
					update_wifi.pktNum= ((uint32)update_wifi.Xcycle_upd*256) + (uint32)update_wifi.packetno;


					/*packetno is start from 1, thus we check the header in the first packet*/
					/*the 1st packet of Upgrade FW is header,  include:  */
					
					/*|  3BYTE	 |	  4BYTE (Low-->High)   |   1 BYTE      |  120 BYTE |*/
					/*|  Header     |		 Length                  |      Type      |  PADDING |*/
					
					if (update_wifi.pktNum== 1)
					{	
					//Printf_High("first pack.....\n");
					//	Printf_High( "pData[0] :%2X,pData[1]:%2X,pData[2]:%2X;UartUpgHeader[0]:%2X;UartUpgHeader[1]:%2X;UartUpgHeader[2]:%2X \n",\
					//				pData[0],pData[1],pData[2],UartUpgHeader[0],UartUpgHeader[1],UartUpgHeader[2]);
						/*compare first 5 bytes */
						if(!memcmp(pData, UartUpgHeader, UART_FW_HEADER_DATA_SIZE))
						{
							DEBUG_XMOD("first pack compare ok..\n");
							//UpgSize  = *((uint32 *)(pData+UART_FW_HEADER_DATA_SIZE));
							update_wifi.UpgSize  =	(((uint32)(*(pData + UART_FW_HEADER_DATA_SIZE))) 	     | 
													((uint32)(*(pData + UART_FW_HEADER_DATA_SIZE + 1)) << 8) |
													((uint32)(*(pData + UART_FW_HEADER_DATA_SIZE + 2)) << 16)|
													((uint32)(*(pData + UART_FW_HEADER_DATA_SIZE + 3)) << 24));

							/*The 8st Byte of UartUpgHeader[] is update Type */
							//第7B 数据是更新sta  ap  还是recovery  固件
							update_wifi.type     = pData[UART_FW_HEADER_DATA_SIZE + UART_FW_HEADER_LEN_SIZE];
							//update_wifi.type     = UART_FlASH_UPG_ID_AP_FW;
						}
					}
					else  
					{
						DEBUG_XMOD("second pack\n");
						if ((update_wifi.type == 0) || (update_wifi.type >= UART_FlASH_UPG_ID_MAX))
								goto xmod_CAN;
						
						DEBUG_XMOD("flash writeing....\n");
						
						IoT_Xmodem_Update_FW_Start(); /*Disable Uart Rx Interrupt*/
						flash_write_result = spi_flash_update_fw(UART_FlASH_UPG_ID_AP_FW, \
												(update_wifi.pktNum-2)*bufsz, pData, (uint16)count);
						IoT_Xmodem_Update_FW_Stop();  /*Restore Uart Rx Interrupt*/

						if(flash_write_result != 0)
							update_wifi.flash_error_count++;

						update_wifi.len += count;
			
					}
#else 
					ret = fwrite(pData,DATAELEMENT,count,fp);
					if(ret != count){
						DEBUG_XMOD("fwritelen = %d;recbufflen=%d \n",ret,count);
						goto xmod_CAN;
						}
						
					foffset += ret;
					fseek(fp,foffset,SEEK_SET);						
#endif
				
				update_wifi.packetno_old = update_wifi.packetno;  //保持上一包数据
				++update_wifi.packetno;
				
				++update_wifi.flashwritepack_cnt;
				update_wifi.retrans = PACKET_RETRY_LIMIT + 1;
				txbuff[0] = XMOD_ACK;
				//Printf_High("packetno:%d, \n",update_wifi.packetno);

				}
				txbuff[0] = XMOD_ACK;
				txbuff[1] = update_wifi.packetno;		//下一包序号
				txbuff[2] = ~update_wifi.packetno;	//下一包 序号补码
				return 3;	
			}
		}
//重发		
retry_xmod:	
		
		DEBUG_XMOD("retry_xmod\n");
		DEBUG_XMOD("packetno:%d\n",update_wifi.packetno);

		flushinput();
		if (--update_wifi.retrans <= 0)
			goto xmod_CAN;
		
		txbuff[0] = XMOD_NAK;
		txbuff[1] = update_wifi.packetno;		//下一包序号
		txbuff[2] = ~update_wifi.packetno;	//下一包 序号补码
		return 3;

xmod_CAN:
	
	    DEBUG_XMOD("CAN end rec \n");
		flushinput();	
#ifndef _WIN32		
		cnmTimerStopTimer(&updatetimerout);
#else
		fclose(fp);
#endif
		updatewifi_int();
	 	txbuff[0] = XMOD_CAN;
        txbuff[1] = XMOD_CAN;		
        txbuff[2] = XMOD_CAN;
		return 3;

}
#ifndef _WIN32
//移动更新的固件数据  返回已经移动的字节数
// -1 表示失败flash里的固件已经损坏
int flashcopy_Update_FW(VOID)
{
	unsigned char xbuff[150];  /*only support 128Byte xmodem*/
	unsigned char packetno = 1;
	long i,len = 0;
	
	uint32 pktNum = 0;		  /*if packetno >255 , use this parameter*/
	uint8  Xcycle = 0;        /*if packetno >255 , use this parameter*/
    uint8  flash_write_result;
    uint32 offset;  //flash offset
	//uint8* pData	= xbuff;
	if(update_wifi.flashwritepack_cnt < 10)
		return 0;
	//Printf_High("IoT_Xmodem_Update_FW\n");
	
	IoT_Xmodem_Update_FW_Start(); /*Disable Uart Rx Interrupt*/
	for(i=0;i < update_wifi.flashwritepack_cnt; i++)
	{

						/*if packetno > 255 , it will overflow, thus , 
					we must use pktNum as the flash write offset */
				if((packetno == 0) && (pktNum % 256 == 255))
					{
						Xcycle++;
					}
					pktNum = ((uint32)Xcycle*256) + (uint32)packetno;
					offset = (pktNum-2)*TXBUFSIZE;

					//读取		
				spi_flash_read(FLASH_OFFSET_AP_FW_START+offset, xbuff, TXBUFSIZE);
					//默认每次都是128b 的bin 字节
				flash_write_result = spi_flash_update_fw(update_wifi.type, \
											offset, xbuff, (uint16)TXBUFSIZE);
	
				if(flash_write_result != 0)
							goto flashupdatefailt;

				len += TXBUFSIZE;
				++packetno;		
				//Printf_High("copened packno :%d\n",packetno);
			}
	    Printf_High("copy done reboot !!\n ");
		IoT_uart_output( AT_CMD_REBOOT, sizeof(AT_CMD_REBOOT)-1);
		IoT_Xmodem_Update_FW_Stop();  /*Restore Uart Rx Interrupt*/
		Sys_reboot();
		Sys_reboot();
		return 	len;										
flashupdatefailt:	
		Printf_High("flash copy error \n");
		return -1;
	
}
#endif

#endif // end xmode  


