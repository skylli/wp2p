/*******************************************************************************

    This file is part of the up2p.
    Copyright wilddog.com
    All right reserved.

    File:    up2pa.h

    No description

    TIME LIST:
    CREATE  skyli   2017-05-06 13:47:13

*******************************************************************************/
#ifndef _up2pa_h_
#define _up2pa_h_

#ifdef		__cplusplus
extern "C" {
#endif

#include "up2p.h"

/*
 * ��ʼ��
 * sport ���ذ󶨶˿� sname ��������������IP dport �������˿� cport�ͻ��˵Ķ˿�
 */
int up2pa_init(u16 sport, const char *sname, u16 dport, u16 cport, u32 mac0, u32 mac1);

/*
 * ж��
 */
int up2pa_deinit();

/*
 * ���ͼ������״̬�����ݰ�, ��ѯ����״̬֮ǰ�ȵ���
 */
int up2pa_send_online(u32 dev0, u32 dev1);

/*
* �������״̬
* ����ֵ 0 ������ 1 ���� -1 ����������Ӧ
*/
int up2pa_check_online(u32 dev0, u32 dev1);

/*
 * ��������״̬
 */
int up2pa_clear_online(u32 dev0, u32 dev1);

/*
 * ����Զ��ָ��
 * cmd ָ����� param ָ������� len �������ݳ���
 * û�и������� param ����ΪNULL, lenΪ0
 */
int up2pa_send_cmd(u32 dev0, u32 dev1, u32 cmd, const char *param, int len);

/*
 * ���ʹ�������Զ��ָ��
 * idx ������ cmd ָ����� param ָ������� len �������ݳ���
 * û�и������� param ����ΪNULL, lenΪ0
 */
int up2pa_send_idx_cmd(u32 dev0, u32 dev1, u16 idx, u32 cmd, const char *param, int len);

/*
 * ���ͼ���Զ��ָ��
 * token ���� cmd ָ����� param ָ������� len �������ݳ��� 
 * û�и������� param ����ΪNULL, lenΪ0
 */
int up2pa_send_data_cmd(u32 dev0, u32 dev1, u32 key0, u32 key1, u32 token, u32 cmd, const char *param, int len);

/*
 * ��ȡ����ָ��
 * data ���ջ��� maxlen ��������󳤶�
 * ����ֵ �������ݵĳ��� ����Ҳ����Ϊ0 -1 ������
 */
int up2pa_recv_cmd(u32 dev0, u32 dev1, u32 *cmd, char *data, int maxlen);

/*
 * ��ȡ�������ķ���ָ��
 * idx ������ data ���ջ��� maxlen ��������󳤶�
 * ����ֵ �������ݵĳ��� ����Ҳ����Ϊ0 -1 ������
 */
int up2pa_recv_idx_cmd(u32 dev0, u32 dev1, u16 idx, u32 *cmd, char *data, int maxlen);

/*
 * �������ȴ�ָ����ָ��
 * cmd �ȴ���ָ�� data ���ջ��� maxlen ��������󳤶�
 * ����ֵ �������ݵĳ��� ����Ҳ����Ϊ0 -1 ������
 */
int up2pa_wait_cmd(u32 dev0, u32 dev1, u32 cmd, char *data, int maxlen);

/*
 * ��ȡ���ؼ���ָ��
 * data ���ջ��� maxlen ��������󳤶�
 * ����ֵ �������ݵĳ��� ����Ҳ����Ϊ0 -1 ������
 */
int up2pa_recv_data_cmd(u32 dev0, u32 dev1, u32 key0, u32 key1, u32 token, u32 *cmd, char *data, int maxlen);

/*
 * �������ȴ�ָ���ļ���ָ��
 * cmd �ȴ���ָ�� data ���ջ��� maxlen ��������󳤶�
 * ����ֵ �������ݵĳ��� ����Ҳ����Ϊ0 -1 ������
 */
int up2pa_wait_data_cmd(u32 dev0, u32 dev1, u32 key0, u32 key1, u32 token, u32 cmd, char *data, int maxlen);

/*
 * ���÷���������
 */
int up2pa_set_host(u32 sip, u16 sport);

/*
 * �߽籣����ʼ
 * �ڶ��߳���ͨѶ����ʹ�ô˺�����Ϊ��ʼ
 */
void up2pa_border_begin();

/*
 * �߽籣������
 * �ڶ��߳���ͨѶ����ʹ�ô˺�����Ϊ����
 */
void up2pa_border_end();

/*
 * ������Ϣ���, 0Ϊ�ر�, ����0Ϊ���
 */
extern int up2pa_debug_level;



#ifdef __cplusplus
}
#endif

#endif
