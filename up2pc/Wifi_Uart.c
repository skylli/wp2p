/*******************************************************************************

    This file is part of the wifi to uart communication.
    Copyright wilddog.com
    All right reserved.

    File:    wifi_uart.c

    No description

    TIME LIST:
    CREATE  skyli   2017-05-07  

*******************************************************************************/
#include "../up2p.h"
// Íù´®¿ÚÐ´Êý¾Ý
int serial_write(const char *data, int len)
{
	int i=0;
	printf("\nserial_write : ");
	for(i=0;i<len;i++){
		printf("[%02x]",data[i]);
	}
	printf("\nserial_write : %s\n",data);
	return len;
}

 int serial_read(char *data, int len)
{
	printf("serial read : ok\n");
	memcpy(data,"serial read : ok\n",strlen("serial read : ok\n")+1);
	return strlen(data);
}

