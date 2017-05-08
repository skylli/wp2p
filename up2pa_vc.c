/*******************************************************************************

    This file is part of the up2p.
    Copyright wilddog.com
    All right reserved.

    File:    up2pa_vc.c

    No description

    TIME LIST:
    CREATE  skyli   2017-05-06 13:47:30

*******************************************************************************/

//#include "up2pa.h"
#include <stdio.h>
#include <stdlib.h>
#include "ulink.h"

static u16 local_port, server_port, client_port;
static char * server_name;
static u32 dev0, dev1, mac0, mac1;
static u32 key0, key1, token;

static char *filename;
static FILE *inputfile;
static FILE *outputfile;

static char recvbuf[1024];
static char sendbuf[1024];
static I2C_cust_ I2C_cust;



// port
int main(int argc, const char *argv[])
{
	char key[32];
	ULINK *ulink;
	int ret;

	if(argc < 4)
	{
		printf("up2pa host_ip myid devid\n");
		return 0;
	}
	ulink_test("0000000C4326605A");

    //ulink_check();
	if (argc > 4)
	{

	 ulink_updata(argv[3],argv[4]);
	}
	while(1)
	{
		delay_ms(500);
	}

}
