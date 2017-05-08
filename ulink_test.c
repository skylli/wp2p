/*******************************************************************************

    This file is part of the project.
    Copyright wilddog.com
    All right reserved.

    File:    ulink_test.c

    No description

    TIME LIST:
    CREATE  skyli   2014-09-01 20:08:23

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include "ulink.h"

#ifdef __ANDROID__
#include <android/log.h>
#define LOG    "kenjni" // 这个是自定义的LOG的标识
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG,__VA_ARGS__) // 定义LOGI类型
#else
#define LOGI debug_log
#endif
static char recvbuf[1024];
static char sendbuf[1024];


//获取文件的长度 大小
static FILE *finput = NULL;
static long filesize;			//文件的大小
static u8 *filebuff = NULL;
static long filecurr;			//当前文件位置

long copybinfile(const char* filename )
{
	//long filesize =0;
	long filelen = 0;
	//读二进制方式打开
	finput =  fopen(filename,"rb");
	
	if( finput == NULL)   
		{
		perror("vm101_sta.bin:");
		return -1;
		//exit(EXIT_FAILURE);		//有错直接退出
	}
    fseek( finput, 0L, SEEK_END );
    filesize = ftell(finput);
	printf("file :%s;size :%d\n",filename,filesize);
	filebuff = (u8 *)malloc(filesize);
	if(filebuff == NULL)
	{
		printf("malloc error !!\n");
		goto closefile;
	}
	fseek( finput, 0, SEEK_SET );  //从文件头开始copy
	filelen = fread(filebuff,1,filesize,finput);
	if(filelen < filesize )
		printf("read file len error filelen:%d;filesize:%d\n",filelen,filesize);
	filecurr = 0;

closefile:
	if(fclose(finput) !=0 )
		perror("fclose:");
	return filelen;
}
extern int up2pa_send_online(u32 dev0, u32 dev1);
extern int up2pa_check_online(u32 dev0, u32 dev1);
void ulink_check(void)
{
	ULINK *ulink;
	char key[32];
	int ret;
	ulink_debug(0);
	
//	strcpy(key, "0000000000000000");
	ulink_init("p2p.wilddog.cn", "0102030405060708");
	while (1)
	{
//		ulink = ulink_open("1234567890123456", key);
		up2pa_send_online(0,0);
		delay_ms(1000);
		ret = up2pa_check_online(0, 0);
//		ulink_close(ulink);
	}


}
void ulink_updata(const char *dev,const char *vm10x_file_name)
{
	ULINK *ulink;
	char key[32];
	int ret;

	ulink_debug(0);
	ulink_init("p2p.wilddog.cn", "0102030405060708");
#if 1	
	ret = ulink_config(dev, "my wifi", "12345678", ULINK_AUTH_WPA1PSKWPS2PSK, &key[0]);
	if(ret != ULINK_ERR_NONE)
	{
		printf("ulink_config err = %d\n", ret);
		goto _err;
	}
#else
	strcpy(key, "0000000000000000");
#endif
	ulink = ulink_open(dev, key);
	if(ulink == NULL)
	{
		printf("ulink_open faile\n");
		delay_ms(1000);
		goto _err;
	}
//	while (1)
	{

		//查询固件版本
		ulink_cmd_send(ulink,CMD_WIFISDKVER_QURY, NULL,0);
		memset(recvbuf, 0, MAX_SERIAL_SIZE);
		ret = ulink_cmd_wait(ulink, CMD_WIFISDKVER_QURY_ACK, recvbuf, MAX_SERIAL_SIZE);
		printf("CMD_WIFISDKVER_QURY_ACK ret = %d buf = %s\n", ret, recvbuf);
		delay_ms(1000);
		//固件载入内存
		ret = copybinfile(vm10x_file_name);
		if(ret > 0 )
		{
			ulink_update_vm10x(ulink,(char*)&filebuff[0],filesize);			
		}
	}
_err:
	ulink_deinit();
}

void ulink_test(const char *dev)
{
	int ret;
	ULINK *ulink;
	char outkey[32];

	ulink_debug(10);

	ulink_init("p2p.wilddog.cn", "0102030405060708");
												   
	ret = ulink_config(dev, "my wifi", "12345678", ULINK_AUTH_WPA1PSKWPS2PSK, &outkey);
	if(ret != ULINK_ERR_NONE)
	{
		printf("ulink_config err = %d\n", ret);
		goto _err;
	}

	ulink = ulink_open(dev, outkey);

	if(ulink == NULL)
	{
		printf("ulink_open faile\n");
		delay_ms(1000);
		goto _err;
	}

	ulink_border_begin();

	// 在线检查
	ulink_send_online(ulink);
	delay_ms(500);
	ret = ulink_check_online(ulink);
	printf("online ret = %d\n", ret);
	delay_ms(500);


	while(1)
	{
#if 1
		// 发送串口命令，并读取串口数据
		strcpy(sendbuf, "G28"); //回到原点
		ulink_cmd_send(ulink, CMD_TXRX_SERIAL, sendbuf, strlen(sendbuf));
		ret = ulink_cmd_wait(ulink, CMD_TXRX_SERIAL_ACK, recvbuf, MAX_SERIAL_SIZE);
		if (ret >= 0)
		{
//			if (recvbuf[2] != 'V')
			{
 				printf("CMD_TXRX_SERIAL_ACK ret = %d buf = %s\n", ret, recvbuf);
			}
		}
		else
		{
            printf("CMD_TXRX_SERIAL_ACK error\n");
		}
#endif

#if 0
		//查询固件版本
		ulink_cmd_send(ulink,CMD_WIFISDKVER_QURY, NULL,0);
		memset(recvbuf, 0, MAX_SERIAL_SIZE);
		ret = ulink_cmd_wait(ulink, CMD_WIFISDKVER_QURY_ACK, recvbuf, MAX_SERIAL_SIZE);
		printf("CMD_WIFISDKVER_QURY_ACK ret = %d buf = %s\n", ret, recvbuf);


		//固件载入内存
		ret = copybinfile("MT7681_sta_header.bin");
		if(ret > 0 )
			{
			ulink_update_vm10x(ulink,filebuff,ret,recvbuf[0]);
			ulink_border_end();
		}
#endif

	}
		ulink_border_end();
		ulink_close(ulink);
_err:
	ulink_deinit();
}
