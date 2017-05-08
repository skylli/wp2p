/*******************************************************************************

    This file is part of the up2p.
    Copyright wilddog.com
    All right reserved.

    File:    up2pa.h

    No description

    TIME LIST:
    CREATE  skyli   2014-08-20 20:17:45

*******************************************************************************/

#include "up2p.h"
#include "aes.h"

/*
 * ���ݼ���
 * src Դ���� dst Ŀ������ len Դ���ݳ��� key0 key1 ��Կ
 * ���ؼ��ܺ�����ݳ���
 * ���src��Ϊ16�ֽڵ�������,��0�����
 */
int data_enc(const char *src, char *dst, int len, u32 key0, u32 key1)
{
	//	aes_context ctx;
	u32 newkey[4];
	u8  tmp_input[16];
	int count;

	newkey[0] = key0;
	newkey[1] = key1;
	newkey[2] = key0;
	newkey[3] = key1;

	//aes_set_key(&ctx, (unsigned char *)newkey, 128);
	
	count = 0;
	while(len >= 16)
	{
		//aes_encrypt(&ctx, (unsigned char*)(src + count), (unsigned char*)(dst + count));
		AES128_ECB_encrypt(src + count, newkey, dst + count);
		count += 16;
		len -= 16;
	}
	if(len > 0)
	{
		memset(tmp_input, 0, 16);
		memcpy(tmp_input, &src[count], len);
		//aes_encrypt(&ctx, tmp_input, (unsigned char*)(dst + count));
		AES128_ECB_encrypt(tmp_input, newkey, dst + count);
		count += 16;
	}

	return count;
}

/*
 * ���ݽ���
 * src Դ���� dst Ŀ������ len Դ���ݳ��� key0 key1 ��Կ
 * ���ؼ��ܺ�����ݳ���
 * ���src��Ϊ16�ֽڵ�������,��0�����
 */
int data_dec(const char *src, char *dst, int len, u32 key0, u32 key1)
{
//	aes_context ctx;
	u32 newkey[4];
	u8  tmp_input[16];
	int count;

	newkey[0] = key0;
	newkey[1] = key1;
	newkey[2] = key0;
	newkey[3] = key1;

	//aes_set_key(&ctx, (unsigned char *)newkey, 128);
	
	count = 0;
	while(len >= 16)
	{
		//aes_decrypt(&ctx, (unsigned char*)(src + count), (unsigned char*)(dst + count));
		AES128_ECB_decrypt(src + count, newkey, dst + count);
		count += 16;
		len -= 16;
	}

	return count;
}
