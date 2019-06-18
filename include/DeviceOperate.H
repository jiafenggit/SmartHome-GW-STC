#ifndef __DeviceOperate_H__
#define __DeviceOperate_H__
#include <Data.H>

void func_reset();

void func_back_devices_list_to_net();

void func_reset_password(struct Long_Packet temp);

unsigned int func_flash_read_start_block_addr();

struct Long_Packet func_change_device(struct Long_Packet temp);

void func_save_device_info(struct Long_Packet temp);

struct Long_Packet func_back_device_state(struct Long_Packet temp);
struct Long_Packet func_back_sensor_state(struct Long_Packet temp);
struct Long_Packet func_flash_read_dev_info_by_pac(unsigned char *p);

#endif