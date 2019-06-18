#include <DeviceOperate.H>
#include <FlashOperate.H>
#include "Read_MCU_Info.H"
#include <TimeAndDelay.H>
#include <NetWork.H>
#include <IOHP.H>
#include <AT24C16.H>
#include <IPA_EEPROM.H>
#include <IOSetting.H>
#include <Data.H>

/***设备初始化密码14402(加密后)***/
char code PassWord[17]={0x32,0x36,0x30,0x65,0x62,0x33,0x37,0x66,0x39,0x62,0x61,0x38,0x36,0x62,0x66,0x34,0x00};

void func_reset_password(struct Long_Packet temp)
{
	IAP_EEPROM_Erase_Sector(0);
	Delay100ms();
	IAP_EEPROM_Write_Strings(0,&temp.TAG[0]);
	BackNetLongPacket(temp);
}
void func_reset()
{
	int i;
	IAP_EEPROM_Erase_Sector(0);
	Delay100ms();
	for(i=0;i<sizeof(PassWord);i++)
	{
		IAP_EEPROM_Write_Byte(i,PassWord[i]);
		}
	func_flash_reset_flash();
}

void func_save_device_info(struct Long_Packet temp)
{
	union{
	struct Long_Packet a;
	unsigned char b[46];
	}exchange;
	exchange.a = temp;
	Write_Block_24C16(temp.SAC+2,&exchange.b[0],46);
}


void func_back_devices_list_to_net()
{  
	if(DeviceCount>0)
	{
		unsigned int Last=0,Next=0;
		int i;
		unsigned char number = DeviceCount; 
		i=func_flash_read_start_block_addr();
		NetLongPacket=func_flash_read_devices_info(&Last,i,&Next);
		NetLongPacket.Other[2]=DeviceCount;
		BackNetLongPacket(NetLongPacket);
		number--;
		if(number)
		{
			for(i=0;i<number;i++)
			{
				while(!Flag_Get_Net_Long_Packet);
				Flag_Get_Net_Long_Packet=0;	
				NetLongPacket=func_flash_read_devices_info(&Last,Next,&Next);
				NetLongPacket.Other[2] = i;
				BackNetLongPacket(NetLongPacket);
			}
		}
	}
	else
	{
		unsigned int Last=0,Next=0; 
		NetLongPacket=func_flash_read_devices_info(&Last,func_flash_read_start_block_addr(),&Next);
		NetLongPacket.Other[2]=0;
		BackNetLongPacket(NetLongPacket);
		while(!Flag_Get_Net_Long_Packet);
		Flag_Get_Net_Long_Packet=0;
	}
}
struct Long_Packet func_change_device(struct Long_Packet temp)
{
	switch(temp.BCF)
	{
		case BUS_Class_RS485:if(RS485_BUS_Send_LongPacket(temp,0))
				{
					RS485_TimeOut = 0;
					while(!((Flag_Get_RS485_Long_Packet)||(RS485_TimeOut==5)));//0.5秒超时
					if(Flag_Get_RS485_Long_Packet)
					{
						Flag_Get_RS485_Long_Packet=0;		
						temp = RS485_LongPacket; 
						temp.Other[0] = State_Device_OnLine;
						func_save_device_info(temp);
					}
					else
					{
					RS485_BUS_Free();
					temp.Other[0]=State_Device_OffLine;
					temp.CMD = State_Device_OffLine;
					func_save_device_info(temp);
					}
				}
				else
				{
					RS485_BUS_Free();
					temp.Other[0] = State_Device_OffLine;
					temp.CMD = State_Device_OffLine;
					func_save_device_info(temp);
		
				}break;
		default :break;
	}
	return temp;
}

struct Long_Packet func_back_device_state(struct Long_Packet temp)
{
	unsigned int a,b;
	return func_flash_read_devices_info(&a,temp.SAC,&b);
}

struct Long_Packet func_back_sensor_state(struct Long_Packet temp)
{
	switch(temp.BCF)
	{
		case BUS_Class_RS485:
				if(RS485_BUS_Send_LongPacket(temp,NOT_FREE))
				{
					RS485_TimeOut = 0;
					while(!((Flag_Get_RS485_Long_Packet)||(RS485_TimeOut==5)));//0.5秒超时
					if(Flag_Get_RS485_Long_Packet)
					{
						Flag_Get_RS485_Long_Packet=0;
						temp = RS485_LongPacket;
					}
					else
					{
						temp.Other[0]=State_Device_OffLine;
					}
				}
				else
				{
					RS485_BUS_Free();
					temp.Other[0] = State_Device_OffLine;
					func_save_device_info(temp);
		
				};break;
		default:temp.Other[0] = State_Device_OffLine;break;

	}
	return temp;
}
