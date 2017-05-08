/*******************************************************************************

    This file is part of the up2p.
    Copyright wilddog.com
    All right reserved.

    File:    up2ps.h

    No description

    TIME LIST:
    CREATE  skyli   2014-08-13 10:57:52

*******************************************************************************/
#ifndef _up2ps_h_
#define _up2ps_h_

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_USER 65536

#include "up2p.h"

typedef struct{
	u32 dev0;		// 设备名
	u32 dev1;
	u32 ip;		// 设备IP
	u16 port;		// 设备端口
	u32 time;
} DEVINFO;

extern DEVINFO *devinfolist;

int ctrl_init(void);

int user_add(u32 dev0, u32 dev1, u32 ip, u16 port);
int _send(const char *data, int len, u32 ip, u16 port);
DEVINFO *user_find(u32 dev0, u32 dev1);

#ifdef __cplusplus
}
#endif

#endif
