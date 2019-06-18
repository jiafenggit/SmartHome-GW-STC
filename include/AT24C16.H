#ifndef __AT24C08_H__
#define	__AT24C08_H__

void Init_24C16();
/* AT24C08初始化函数 */

unsigned char Read_Byte_24C16(unsigned int address);
/* 从AT24C08中读取一字节数据 */

void Write_Byte_24C16(unsigned int address,unsigned char info);
/* 向AT24C08中写入一字节数据 */
void Write_Block_24C16(unsigned int address,unsigned char* info,unsigned char length);

#endif