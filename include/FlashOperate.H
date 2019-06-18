#ifndef __FlashOperate_H_
#define __FlashOperate_H_

#include<Data.H>
extern unsigned char DeviceCount;
unsigned int ReadStartBlockAddr();


struct Long_Packet func_flash_read_devices_info(unsigned int *LastBlockAddr,unsigned int Address,unsigned int *NextBlockAddr);
void func_flash_auto_build_block();
void func_flash_reset_flash();
void func_flash_delete_device_info(unsigned int OrderInfoAddr);
struct Long_Packet func_flash_add_device_info(struct Long_Packet New);
struct Long_Packet func_flash_read_dev_info_by_pac(unsigned char *p);
//void echoint(unsigned int a);

#endif