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
#define MAXUPDATETIMOUT_CNT	120	//1m = 60 * 2 * 1 = 120     // 1������û�и��°� ���Ƴ�����
#define UPDTIMOUT_INV		500//500MS
TIMER_T	      updatetimerout;      //wifi update reading  timerout  500ms peer
#else 
#define Printf_High   printf

#endif


typedef struct{
	u8 updateting_sta;	//��ǰ�Ƿ��ڸ���״̬
	u8 packetno;	//��ǰ�����
	u8 packetno_old;//��һ����� 
	u8 retrans ;	//�ط�������
	u8	type;
    u8 updaetimout_cnt; //�ȴ����°� ������
	
	long flashwritepack_cnt;
 	int flash_error_count ;		//flash ����
    int UpgSize  ;
 	int len;		//��ǰд��flash ���ֽ���

	u8  Xcycle_upd ;		  /*if packetno >255 ,  flash ��ҳ*/
	u32 pktNum;		  /*if packetno >255 , use this parameter*/

	u8 Hap_Lsta_fig;//��ǰΪap ����   0  ��ǰ��sta ����

} wifiupdate_stc;
static wifiupdate_stc update_wifi;

//u8 retrans = PACKET_RETRY_LIMIT;//�ط�������
//static 	int flash_error_count = 0;		//flash ����
//static int    UpgSize  = FLASH_UPG_FW_SIZE;



int flashcopy_Update_FW(VOID);



//��ʼ��wifi ���µĽṹ��
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
	update_wifi.updateting_sta	=0;	//��ʼ����

	update_wifi.Xcycle_upd =0;
	update_wifi.pktNum	   =0;
	update_wifi.updaetimout_cnt =0;
	/*
	spi_flash_read((UINT32)MODEADDR_, &update_wifi.Hap_Lsta_fig, 1); //�жϵ�ǰʹ�õ����Ǹ�����Ĺ̼�
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

//��ʼ�� Զ�̸��³�ʱʱ��
void updtim_int(void)
{ 
	
	Printf_High("updtim_int \n");
	cnmTimerInitTimer(&updatetimerout,updatetim_calback,0,0);
}
#endif

//��ʱ���������� �յ�һ���̼���������
void updaetimout_cnt_int(void)
{
	update_wifi.updaetimout_cnt =0;
}
//׼������
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
	update_wifi.updateting_sta	=1;	//��ʼ����
	
	update_wifi.pktNum	   =0;
	update_wifi.Xcycle_upd =0;

#ifndef _WIN32
		cnmTimerStartTimer(&updatetimerout, UPDTIMOUT_INV);
#else 
		creatnewbinfile("copyfile.bin");
	
#endif

	//spi_flash_read((UINT32)MODEADDR_, &update_wifi.Hap_Lsta_fig, 1); //�жϵ�ǰʹ�õ����Ǹ�����Ĺ̼�
}
int isrunning_APcode(void)
{
	return (update_wifi.Hap_Lsta_fig);
}
//���ڿ�ʼ�׶�
int  updatestar_wifi(void)
{
	return (update_wifi.updateting_sta < 2);
}
//���ڴ���������ݽ׶�
int  updateing_wifi(void)
{
	return (update_wifi.updateting_sta > 0);
}
//���ø��±�ʶ
int setupdateing_wifi(void)
{
	update_wifi.updateting_sta =2;
	return  update_wifi.updateting_sta;
}
#ifdef _WIN32
// ����һ���ļ�
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
	//����pack ���
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
//����128B
//���ǵ����紫��Ķ����Ƚ����ػ�Ӧÿһ����ackʱӦ����д�� ��һ������ź��䲹��
//
int Xmodem_Update_FW(u8 const *xbuff,u8 *txbuff)
{
	//unsigned char xbuff[150];  /*only support 128Byte xmodem*/
	int bufsz; //crcf  У��
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
				//128����
				updaetimout_cnt_int();
				DEBUG_XMOD("XMOD_SOH.\n");
 				bufsz = 128;
 				goto start_recv;
 			case XMOD_STX:
				
 				bufsz = 1024;
				updaetimout_cnt_int();
 				goto start_recv;
 			case XMOD_EOT:
 				//�������
				Printf_High("XMOD_EOT.\n");
 				flushinput();
				
 				txbuff[0] = XMOD_OK;
				txbuff[1] = XMOD_OK;		//��һ�����
				txbuff[2] = XMOD_OK;	//��һ�� ��Ų���
				
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
				//ԭ���Ǹ���ap ����Ҫ����
#ifndef _WIN32
					
				if(update_wifi.type != UART_FlASH_UPG_ID_AP_FW)
						flashcopy_Update_FW();
#endif
				txbuff[0] = XMOD_OK;
				txbuff[1] = XMOD_OK;		//��һ�����
				txbuff[2] = XMOD_OK;	//��һ�� ��Ų���
				//��ֹ��һ�����ݵĵ���
				updatewifi_int();	
				return 3;
				
 			case XMOD_CAN:
 				//��ֹ����
 				 goto xmod_CAN;
 				break;
 			default:
				//�ط�
				goto retry_xmod;
 				break;
 		}
 		
	

start_recv:
		Printf_High("start_recv\n");
		
		if(update_wifi.flash_error_count != 0){
			    Printf_High("flash_error_count error\n");
				goto xmod_CAN;
			}
		if(update_wifi.packetno_old == xbuff[1]) //�յ���һ������
			{
			if (--update_wifi.retrans <= 0)		//�յ�̫����һ������Ҳ����ֹ
 					goto xmod_CAN;
				Printf_High("rec old pack..\n");
				txbuff[0] = XMOD_ACK;
				txbuff[1] = update_wifi.packetno;	//��һ�����
				txbuff[2] = ~update_wifi.packetno;	//��һ�� ��Ų���
				return 3;
			}
	    crc_cal = calcrc(&xbuff[TXMODEBASE], TXBUFSIZE);
		crc_buf_add = TXBUFSIZE+TXMODEBASE;
		buf_crc  = xbuff[crc_buf_add+3]<<24|xbuff[crc_buf_add+2]<<16|xbuff[crc_buf_add+1]<<8|xbuff[crc_buf_add+0];
		Printf_High("crc_cal :%d; buf_crc:%d\n",crc_cal,buf_crc);
		
		Printf_High("packetno:%d,xbuff[1]:%d \n",update_wifi.packetno,xbuff[1]);
		//printf("packetno=%2X;xbuff[0] =%2X ;xbuff[1] =%2X ;xbuff[2] =%2X ;xbuff[131] =%2X ;xbuff[132] =%2X;xbuff[133] =%2X;\n",\
		//						packetno,xbuff[0],xbuff[1],xbuff[2],xbuff[131],xbuff[132],xbuff[133]);
		//�жϰ�ͷ //crc У��
		if (xbuff[1] == (unsigned char)(~xbuff[2]) &&\
			(xbuff[1] == update_wifi.packetno || xbuff[1] == (unsigned char)update_wifi.packetno-1) &&\
			(crc_cal == buf_crc ))
		{
			//�жϰ����
			if (xbuff[1] == update_wifi.packetno)
			{
				int count = update_wifi.UpgSize - update_wifi.len;
				
				pData = &xbuff[3];
				
				if (count > bufsz)
					count = bufsz;
				//�����յ�������
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
							//��7B �����Ǹ���sta  ap  ����recovery  �̼�
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
				
				update_wifi.packetno_old = update_wifi.packetno;  //������һ������
				++update_wifi.packetno;
				
				++update_wifi.flashwritepack_cnt;
				update_wifi.retrans = PACKET_RETRY_LIMIT + 1;
				txbuff[0] = XMOD_ACK;
				//Printf_High("packetno:%d, \n",update_wifi.packetno);

				}
				txbuff[0] = XMOD_ACK;
				txbuff[1] = update_wifi.packetno;		//��һ�����
				txbuff[2] = ~update_wifi.packetno;	//��һ�� ��Ų���
				return 3;	
			}
		}
//�ط�		
retry_xmod:	
		
		DEBUG_XMOD("retry_xmod\n");
		DEBUG_XMOD("packetno:%d\n",update_wifi.packetno);

		flushinput();
		if (--update_wifi.retrans <= 0)
			goto xmod_CAN;
		
		txbuff[0] = XMOD_NAK;
		txbuff[1] = update_wifi.packetno;		//��һ�����
		txbuff[2] = ~update_wifi.packetno;	//��һ�� ��Ų���
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
//�ƶ����µĹ̼�����  �����Ѿ��ƶ����ֽ���
// -1 ��ʾʧ��flash��Ĺ̼��Ѿ���
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

					//��ȡ		
				spi_flash_read(FLASH_OFFSET_AP_FW_START+offset, xbuff, TXBUFSIZE);
					//Ĭ��ÿ�ζ���128b ��bin �ֽ�
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


