#include <IOH_RS485_BUS_Host.H>
#include <Linkage_Control.H>
#include <DeviceOperate.H>
#include <Read_MCU_Info.H>
#include <TimeAndDelay.H>
#include <STC12C5A60S2.H> 
#include <FlashOperate.H>
#include <IPA_EEPROM.H>
#include <IOSetting.H>
#include <AT24C16.H>
#include <NetWork.H>
#include <IOHP.H>
#include <Data.H>

void main()
{
	Uart2Init();	
	Uart1Init();
	Timer0Init();
	func_get_mcu_pac();
	EA = 1;		
	Init_24C16();
	RS485_TimeOut = 0;
	while(RS485_TimeOut<10);//等待网络稳定
	RS485_BUS_Send_Public_Command(COM_SET_BUS_NO_REGIST);
	Cheek_RS485_BUS();
	while(Flag_RS485_BUS_Busy);
	func_flash_auto_build_block();		 
	Buzzer = 0;
	Delay100ms();
	Buzzer = 1;	 
	Delay100ms(); 
	Buzzer = 0;
	Delay100ms();
	Buzzer = 1;	
	while(1)
	{  
		if(Flag_Get_Net_Long_Packet)
		{
			Flag_Get_Net_Long_Packet=0;
			switch(NetLongPacket.CMD)
			{
				case COM_LOGIN 					:	Function_Login(NetLongPacket);break;
				case COM_LOGOUT					:	Function_Logout(NetLongPacket);break;
				case COM_SET_PASWD				:	func_reset_password(NetLongPacket);break;
				case COM_GET_DEV_LIST			:	func_back_devices_list_to_net();break;
				case COM_CON_DEV_CHANGE_STATE	:	BackNetLongPacket(func_change_device(NetLongPacket));break;
				case COM_CON_DEV_CHANGE_TAG		:	BackNetLongPacket(func_change_device(NetLongPacket));break;
				case COM_GET_DEV_STATUS 		:	BackNetLongPacket(func_back_device_state(NetLongPacket));break;
				case COM_GET_SENSOR_STATUS		:	BackNetLongPacket(func_back_sensor_state(NetLongPacket));break;
				case COM_DEL_DEV				:	func_flash_delete_device_info(NetLongPacket.SAC);break;
				case COM_SET_DEV_SEE 			:	BackNetLongPacket(func_change_device(NetLongPacket));break;
				case COM_SET_HOST_RESET			:	Buzzer = 0;BackNetLongPacket(NetLongPacket);func_reset();Delay100ms();Buzzer = 1;break;
		/********传感器联动控制*********/

				case COM_SET_LINKAGE_CONTROL	:	BackNetLongPacket(func_change_device(NetLongPacket));break;
				case COM_READ_LINKAGE_CONTROL	:	BackNetLongPacket(func_back_sensor_state(NetLongPacket));break;
				case COM_DEL_LINKAGE_CONTROL 	:	BackNetLongPacket(func_change_device(NetLongPacket));break;
				case COM_SET_CLIENT_RESET		:	BackNetLongPacket(func_change_device(NetLongPacket));break;

				default:break;
			}
		}

		if(Flag_Get_RS485_Long_Packet)
		{
			Flag_Get_RS485_Long_Packet=0;
			switch(RS485_LongPacket.CMD)
			{
				case COM_CON_DEV_REGIST 		:	RS485_BUS_Send_LongPacket(func_flash_add_device_info(RS485_LongPacket),FREE);break;		 //RS485_BUS_Send_LongPacket(AddDeviceInfo(RS485_LongPacket))
				case COM_CON_DEV_CHANGE_STATE 	: 	func_save_device_info(RS485_LongPacket);break;
				case COM_BACK_DEVICE_INFO		: 	func_save_device_info(RS485_LongPacket);break;
				case COM_LINK_CON_DEV_STA		:	func_linkage_control(RS485_LongPacket);break;
				default :break;
			}
		}
		if(Flag_Request_Send_Host_Address)
		{
			Flag_Request_Send_Host_Address=0;
			RS485_BUS_Send_Host_Address(1);
		}
		if(ReSet_Key == 0)
		{
			Delay100ms();
			if(ReSet_Key==0)
			{
				Buzzer = 0;
				while(ReSet_Key);
				Buzzer = 1;
				Delay100ms();
				Buzzer = 0;
				func_reset();
				Buzzer = 1;
			}
		}
		if(Flag_Get_Net_Short_Packet)
		{
			Flag_Get_Net_Short_Packet=0;
			BackNetShortPacket(NetShortPacket);
		}
		if(Flag_New_Client)
		{	
			Flag_New_Client=0;
		}
	}
}