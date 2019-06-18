#include <IOH_RS485_BUS_Host.H>
#include <Linkage_Control.H>
#include <DeviceOperate.H>
#include <STC12C5A60S2.H>
#include<Read_MCU_Info.H>
#include <FlashOperate.H>
#include <IOHP.H>


void func_linkage_control(struct Long_Packet com)
{
	int i;
	unsigned char PAC[8];
	struct Long_Packet link_temp;
/*****将受控设备PAC从数据包中提取出来*****/
	for(i=0;i<8;i++)
	{
		PAC[i] = com.TAG[i];
	}

/****根据受控设备PAC代码获得设备详细信息****/
	link_temp = func_flash_read_dev_info_by_pac(&PAC[0]);

	switch(link_temp.CMD)	  /***查看结果***/
	{
		case STATE_DEV_NOT_EXIST: com.CMD = link_temp.CMD;break;

	/*****存在并且在线**********/
		case State_Device_OnLine :
				link_temp.CMD = COM_CON_DEV_CHANGE_STATE;
				for(i=0;i<10;i++)
				{
					link_temp.STA[i] = com.TAG[i+8];
				}
				link_temp = func_change_device(link_temp);
				com.CMD = link_temp.CMD;
				break;
		default :com.CMD =ERROR; break;

	}
	/****回馈结果给发起联动控制的传感器****/
	RS485_BUS_Send_LongPacket(com,FREE);	
}