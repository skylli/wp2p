/*******************************************************************************

    This file is part of the up2p.
    Copyright wilddog.com
    All right reserved.

    File:    up2pa.h

    No description

    TIME LIST:
    CREATE  skyli   2014-08-20 20:17:45

*******************************************************************************/
#ifndef _up2pa_crypt_h_
#define _up2pa_crypt_h_

#ifdef __cplusplus
extern "C"{
#endif

/*
 * ���ݼ���
 * src Դ���� dst Ŀ������ len Դ���ݳ��� key0 key1 ��Կ
 * ���ؼ��ܺ�����ݳ���
 * ���src��Ϊ16�ֽڵ�������,��0�����
 */
int data_enc(const char *src, char *dst, int len, u32 key0, u32 key1);

/*
 * ���ݽ���
 * src Դ���� dst Ŀ������ len Դ���ݳ��� key0 key1 ��Կ
 * ���ؼ��ܺ�����ݳ���
 * ���src��Ϊ16�ֽڵ�������,��0�����
 */
int data_dec(const char *src, char *dst, int len, u32 key0, u32 key1);

#ifdef __cplusplus
}
#endif

#endif
