#ifndef __AT24C08_H__
#define	__AT24C08_H__

void Init_24C16();
/* AT24C08��ʼ������ */

unsigned char Read_Byte_24C16(unsigned int address);
/* ��AT24C08�ж�ȡһ�ֽ����� */

void Write_Byte_24C16(unsigned int address,unsigned char info);
/* ��AT24C08��д��һ�ֽ����� */
void Write_Block_24C16(unsigned int address,unsigned char* info,unsigned char length);

#endif