#include <STC12C5A60S2.H>
#include <IOHP.H>
#include <AT24C16.H>
#include <FlashOperate.H>
#include <Data.H>
#include <IOSetting.H>
#include <IOH_RS485_BUS_Host.H>
#include <Read_MCU_Info.H>
#include <TimeAndDelay.H>
#define OverBlockAddrA	0x002A
#define OverBlockAddrB	0x002B
#define	EndBlockAddrA	0x002C
#define	EndBlockAddrB	0x002D
#define StartBlockAddrA	0x002E
#define StartBlockAddrB	0x002F

unsigned char DeviceCount=0;
//unsigned char xdata VAL_HOST_MAC[8]=0;


unsigned int func_flash_read_start_block_addr()
{
	union{
		volatile unsigned char a[2];
		volatile unsigned int b;
	}temp;
	temp.a[0]=Read_Byte_24C16(StartBlockAddrA);
	temp.a[1]=Read_Byte_24C16(StartBlockAddrB);
	return temp.b;
}

//unsigned char* func_flash_get_host_mac()
//{
//    if(VAL_HOST_MAC[0]==0)        //如果MAC数组为0时即为首次获得MAC地址,需进行对数组赋值.
//    {
//        int i;
//        char *p;   
//        p=0xF1;
//        VAL_HOST_MAC[0]=0x01;     //数组第一位为MCU厂商识别号,STC用0x01
//        for(i=0;i<7;i++) //循环7次从RAM中读取设备ID号
//        {
//            VAL_HOST_MAC[1+i]=*p;
//            p++;
//        }
//    }
//    return &MAC[0];        //返回数组首地址
//}
struct Long_Packet func_flash_read_devices_info(unsigned int *LastBlockAddr,unsigned int Address,unsigned int *NextBlockAddr)
{
	union{
		char Temp[46];
		struct Long_Packet Received;
	}temp;
	union{
		unsigned char read[2];
		unsigned int r;
	}Last,Next;
	int i;
	Last.read[0]=Read_Byte_24C16(Address);
	Address++;
	Last.read[1]=Read_Byte_24C16(Address);
	Address++;
	for(i=0;i<46;i++)
	{
		temp.Temp[i]=Read_Byte_24C16(Address++);
	}
	Next.read[0]=Read_Byte_24C16(Address++);
	Next.read[1]=Read_Byte_24C16(Address++);	
	*LastBlockAddr = Last.r;
	*NextBlockAddr = Next.r;
	return temp.Received;

}
//void echoint(unsigned int a)
//{
//	unsigned char b = a;
//	SBUF = a>>8;
//	while(!TI);
//	TI=0;
//	SBUF = b;
//	while(!TI);
//	TI=0;
//}
void func_flash_auto_build_block()
{
	volatile unsigned int LastBlockAddr,OrderInfoAddr,NextBlockAddr;
	struct Long_Packet temp;
//	union{
//		volatile unsigned char b[2];
//		volatile unsigned int a;		
//	}first;
	union{
		unsigned int address;
		unsigned char arry[2];
	}Over;
	union{
		struct Long_Packet temp;
		unsigned char Data[64];
	}out;
	int i;

	OrderInfoAddr = func_flash_read_start_block_addr();		//将第一个存储设备信息的数据块地址设为目标地址
	RS485_BUS_Send_Public_Command(COM_SET_BUS_NO_REGIST);	//485总线发送禁止自动注册的命令
	temp = func_flash_read_devices_info(&LastBlockAddr,OrderInfoAddr,&NextBlockAddr);//读取第一个设备的信息
	temp.CMD = COM_CON_DEV_REGIST;
//	echoint(OrderInfoAddr);
//	echoint(NextBlockAddr);
	out.temp=temp;
	for(i=0;i<46;i++)
	{
		SBUF = out.Data[i];
		while(!TI);
		TI=0;
	}
	Delay100ms();
	while(~temp.Other[0])	//若读出的设备信息中Other[0]==0xFF(State_Device_Delete)即为结束标志,后面没有有效信息可读
	{	
		DeviceCount++;
//		echoint(OrderInfoAddr);

		switch(temp.BCF)	//设备连接方式分类:485总线
		{
			case BUS_Class_RS485:
				if(RS485_BUS_Send_LongPacket(temp,1))
				{
					//该设备存在,保存信息,读取下一个存储过的设备信息并验证
					Write_Byte_24C16(OrderInfoAddr+45,State_Device_OnLine);
					
				}
				else
				{
					Write_Byte_24C16(OrderInfoAddr+45,State_Device_OffLine);//置Other[0]为离线标志
					
				}break;
			default:break;
		}
		LastBlockAddr = OrderInfoAddr;
		OrderInfoAddr = NextBlockAddr;
		temp = func_flash_read_devices_info(&LastBlockAddr,OrderInfoAddr,&NextBlockAddr);
		temp.CMD = COM_CON_DEV_REGIST;
	}
	if(DeviceCount)
	{	
		/*******标记最后一个存储信息的块地址*********/
		Over.address=OrderInfoAddr;
		Write_Byte_24C16(OverBlockAddrA,Over.arry[0]);
		Write_Byte_24C16(OverBlockAddrB,Over.arry[1]);
	//	Buzzer=0;	
	}

	/******没有存储对应信息的设备开始注册********/
	RS485_BUS_Send_Host_Address(0);
	RS485_BUS_Send_Public_Command(COM_SET_BUS_AUTO_REGIST);
}

void func_flash_reset_flash()
{
	union{
		unsigned int num;
		unsigned char arry[2];
		}endAddr;
	unsigned int i;
	for(i=0;i<0x0800;i++)
	{
		Write_Byte_24C16(i,0xFF);	
		
	}

		Buzzer = 1;	 
	Delay100ms(); 
	Buzzer = 0;
	                                                                  
	endAddr.num = 0x07CE ;
	Write_Byte_24C16(EndBlockAddrA,endAddr.arry[0]);
	Write_Byte_24C16(EndBlockAddrB,endAddr.arry[1]);
	Write_Byte_24C16(StartBlockAddrA,0x00);
	Write_Byte_24C16(StartBlockAddrB,0x30);		                                            
	Write_Byte_24C16(OverBlockAddrA,0x00);
	Write_Byte_24C16(OverBlockAddrB,0x30);

	/* 构建循环链表*/

	for(i=0;i<40;i++)	//AT24C16最多存储40个设备信息
	{
	//	char flag=0xFF;
		union{
		unsigned int num;
		unsigned char arry[2];
		}now,next,last;
		last.num=0X0030+(50*(i-1));
		now.num=0X0030+(50*i);
		next.num=0X0030+(50*(i+1));
		Write_Byte_24C16(now.num,last.arry[0]);
		Write_Byte_24C16(now.num+1,last.arry[1]);
		Write_Byte_24C16(now.num+48,next.arry[0]);
		Write_Byte_24C16(now.num+49,next.arry[1]);
		Write_Byte_24C16(now.num+46,State_Device_Delete);
	}
}
void func_flash_delete_device_info(unsigned int OrderInfoAddr)
{
	//设备不存在,删除设备信息!
	/******************************************************************************************************************************************************
	|	A.LastBlockAddr	A.Devices_Info	A.NextBlockAddr	|	B.LastBlockAddr	B.Devices_Info	B.NextBlockAddr	|	C.LastBlockAddr	C.Devices_Info	C.NextBlockAddr	|
	|	假设B无响应,
	|	则需将原先的	A.NextBlockAddr	->	B
	|					B.LastBlockAddr	->	A
	|					B.NextBlockAddr	->	C
	|					C.LastBlockAddr	->	B
	|	改为:
	|					A.NextBlockAddr	->	C
	|					C.LastBlockAddr	->	A	并找到最后一个数据块D的地址,将D.NextInfoAddr -> B,将B记录为最后一个数据块,Flash(0x0004~0x0005)保存最后一个数据块的地址.
	*************************************************************************************************************************************************************/
	union{
	unsigned char arry[2];
	unsigned int num;
	}endAddr;
	union{
	unsigned int num;
	unsigned char arry[2];			
	}now,next,last;
	//char flag=0xFF;
	unsigned int LastBlockAddr,NextBlockAddr;
	func_flash_read_devices_info(&LastBlockAddr,OrderInfoAddr,&NextBlockAddr);
	now.num=OrderInfoAddr;
	next.num=NextBlockAddr;
	last.num=LastBlockAddr;
	//重新构建链表
	endAddr.arry[0]=Read_Byte_24C16(EndBlockAddrA);
	endAddr.arry[1]=Read_Byte_24C16(EndBlockAddrB);
	Write_Byte_24C16(EndBlockAddrA,now.arry[0]);
	Write_Byte_24C16(EndBlockAddrB,now.arry[1]);
	Write_Byte_24C16(endAddr.num+48,now.arry[0]);
	Write_Byte_24C16(endAddr.num+49,now.arry[1]);
	Write_Byte_24C16(last.num+48,next.arry[0]);
	Write_Byte_24C16(last.num+49,next.arry[1]);
	Write_Byte_24C16(next.num,last.arry[0]);
	Write_Byte_24C16(next.num+1,last.arry[1]);
	Write_Byte_24C16(OrderInfoAddr+46,State_Device_Delete);
	DeviceCount--;
}
bit func_flash_mac_compare(unsigned char *p1,unsigned char *p2)
{
	int i;bit r=1;
	for(i=0;i<8;i++)
	{
		if( *(p1+i) != *(p2+i))
		{
			r=0;
			break;
		}
	}
	return r;
}
struct Long_Packet func_flash_add_device_info(struct Long_Packet New)
{
	union{
	unsigned char arry[2];
	unsigned int num;
	}endAddr,StartAddr;
	unsigned int Now,Last,Next;
	struct Long_Packet temp;
	union{
		struct Long_Packet a;
		unsigned char b[46];
	}exchange;
	endAddr.arry[0]=Read_Byte_24C16(OverBlockAddrA);
	endAddr.arry[1]=Read_Byte_24C16(OverBlockAddrB);
	StartAddr.arry[0]=Read_Byte_24C16(StartBlockAddrA);
	StartAddr.arry[1]=Read_Byte_24C16(StartBlockAddrB);
	Now = StartAddr.num;
////	echoint(endAddr.num);
//	echoint(StartAddr.num);
	while(Now != endAddr.num)
	{
//		echoint(Now);
		temp=func_flash_read_devices_info(&Last,Now,&Next);
		//echoint(Next);
		if(func_flash_mac_compare(&temp.PAC[0],&New.PAC[0]))
		{
			Write_Byte_24C16(Now+45,State_Device_OnLine);
			temp.SAC = Now;
			break;
		}
		else
		{
			Last=Now;
			Now=Next;
		}
		
	}
	if(Now==endAddr.num)
	{
/***OverBlock是第一个无信息存储的存储块,应该把新的设备信息存储在OverBlock,
	并根据OverBlock的链表找到下一个Block并把它标记为OverBlock		****/
		New.SAC=endAddr.num; 
//		echoint(endAddr.num);
		New.Other[0] = State_Device_OnLine;
		exchange.a=New;
		Write_Block_24C16(New.SAC+2,&exchange.b[0],46);
		
		StartAddr.arry[0]=Read_Byte_24C16(New.SAC+48);
		StartAddr.arry[1]=Read_Byte_24C16(New.SAC+49);
		
		Write_Byte_24C16(OverBlockAddrA,StartAddr.arry[0]);
		Write_Byte_24C16(OverBlockAddrB,StartAddr.arry[1]);
//		echoint(StartAddr.num);
		DeviceCount = DeviceCount+1;
		temp=New;
	}	
	return temp;
}
struct Long_Packet func_flash_read_dev_info_by_pac(unsigned char *p)
{
	union{
	unsigned char arry[2];
	unsigned int num;
	}endAddr,StartAddr;
	unsigned int Now,Last,Next;
	struct Long_Packet temp;

	endAddr.arry[0]=Read_Byte_24C16(OverBlockAddrA);
	endAddr.arry[1]=Read_Byte_24C16(OverBlockAddrB);
	StartAddr.arry[0]=Read_Byte_24C16(StartBlockAddrA);
	StartAddr.arry[1]=Read_Byte_24C16(StartBlockAddrB);
	Now = StartAddr.num;
	while(Now != endAddr.num)
	{
		temp=func_flash_read_devices_info(&Last,Now,&Next);
		if(func_flash_mac_compare(&temp.PAC[0],p))
		{
			//Write_Byte_24C16(Now+45,State_Device_OnLine);
			//temp.SaveAddr = Now;
			temp.CMD=State_Device_OnLine;
			break;
		}
		else
		{
			Last=Now;
			Now=Next;
		}
		
	}
	if(Now==endAddr.num)
	{
/****没有找到		****/
	temp.CMD=STATE_DEV_NOT_EXIST;
	}	
	return temp;
}	