package com.ulink;

public class UlinkIIC {
	public int clk_pin;	// i2c scl pin �ź�
	public int sda_pin;	// i2c sda pin �ź�
	public int speed;	// i2c �ٶȣ�1,10,40,��λ10k
	public int address;	// i2c ���豸��ַ
	public int w_len;   // д���ݳ���
	public int r_len;   // �����ݳ���
	public byte date[]; // ��/д��������
}
