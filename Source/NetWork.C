/***************NetWork********************
			智能家居控制系统
			主机网络驱动程序
			   2015-02-12 
			   Author:田津坤
			   E-mail:tianjinkun@kftjk.cn
			   www.ihomediy.cn
Note:
	暂时无法支持双工通信,发网络数据包时请注意
	数据包间隔时间!!!
	发送数据包函数:
		void BackNetLongPacket(struct Long_Packet Temp)
			发送43字节设备信息详细数据包
			其中包含{
			char MAC[8];  	//8字节设备物理地址
			char Command;	//1字节数据包控制命令
			int  Device_Class; 	   //2字节设备类型
			char Bus_Class;		//1字节总线类型
			char Name[19];		//19字节设备名称
			char State[10];		//10字节设备状态
			int SaveAddr;		//2字节Flash设备状态存储地址
			char Other[3];		//3字节其他信息
			}
		void BackNetShortPacket(struct Short_Packet Temp)
			发送12字节设备控制信息数据包
			其中包含{
			char MAC[8];  //8字节设备物理地址
			char Command; //1字节数据包控制命令
			char State[10];//10字节设备状态
			int SaveAddr;  //2字节Flash设备状态存储地址
			}
	收到一个43字节设备信息详细数据包后将Flag_Get_Net_Long_Packet
	标志位置1,由软件清零;数据将存入全局变量NetLongPacket中.

	收到一个12字节设备控制信息数据包后将Flag_Get_Net_Short_Packet
	标志位置1,由软件清零;数据将存入全局变量NetShortPacket中.

	*网络模块是通过串口2接入MCU,所以要进行相关设备的初始化
		void Uart2Init(void)		//9600bps@11.0592MHz

******************************************/
#include <Read_MCU_Info.H>
#include <TimeAndDelay.H>
#include <FlashOperate.H>
#include <STC12C5A60S2.H>
#include <IPA_EEPROM.H>
#include <IOSetting.H>
#include <NetWork.H>
#include <IOHP.H>




char NetData[55],NetData_Num=0; 
bit Flag_Get_Net_Short_Packet=0,
	Flag_Get_Net_Long_Packet=0,
	Flag_New_Client=0,
	TXDtemp=0;
  sbit BUZZER = P2^7;
struct Short_Packet NetShortPacket;
struct Long_Packet NetLongPacket;
void Uart2Init(void)		//9600bps@11.0592MHz
{
	AUXR &= 0xF7;		//波特率不倍速
	S2CON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x04;		//独立波特率发生器时钟为Fosc,即1T
	BRT = 0xDC;		//设定独立波特率发生器重装值
	AUXR |= 0x10;		//启动独立波特率发生器
	IE2= 0X01;			//允许串口2中断
}
void UnionShortPacket(char *p)
{
	int i;
	union{
		char Temp[21];
		struct Short_Packet Received;
		}temp;
				for(i=0;i<21;i++)
				{
					temp.Temp[i] = *(p+i);
				}
				NetShortPacket=temp.Received;
				Flag_Get_Net_Short_Packet = 1;
}

void UnionLongPacket(char *p)
{
	int i;
	union{
		char Temp[46];
		struct Long_Packet Received;
		}temp;
				for(i=0;i<46;i++)
				{
					temp.Temp[i] = *(p+i);
				}
				NetLongPacket=temp.Received;
				Flag_Get_Net_Long_Packet = 1;
			//	BUZZER=0;
}

void UART2SendOneByte(unsigned char c)
{
    S2BUF = c;
    while(!TXDtemp);  //若S2TI=0，在此等待
    TXDtemp = 0;	      //S2TI=0
}

void BackNetLongPacket(struct Long_Packet Temp)
{
	int i;
	union{
		struct Long_Packet in;
		char Data[46];
	}temp;
	temp.in = Temp;
	UART2SendOneByte(0X03);
	UART2SendOneByte(0X03);
	UART2SendOneByte(NetData[1]);
	UART2SendOneByte(NetData[2]);
	UART2SendOneByte(0X00);
	UART2SendOneByte(47);
	UART2SendOneByte(TypeLongPacket);
	for(i=0;i<46;i++)
	{
		UART2SendOneByte(temp.Data[i]);
	}
	NetData_Num = 0;
}
void BackNetShortPacket(struct Short_Packet Temp)
{
	int i;
	union{
		struct Short_Packet in;
		char Data[12];
	}temp;
	temp.in = Temp;
	UART2SendOneByte(0X03);
	UART2SendOneByte(0X03);
	UART2SendOneByte(NetData[1]);
	UART2SendOneByte(NetData[2]);
	UART2SendOneByte(0X00);
	UART2SendOneByte(22);
	UART2SendOneByte(TypeShortPacket);
	for(i=0;i<21;i++)
	{
		UART2SendOneByte(temp.Data[i]);
	}
	NetData_Num = 0;
}
bit PasswordCompare(char *p)
{
	bit r = 1;
	int i = 0;
	while(*p)
	{
	//	func_uart_test_echo(*p);
	//	func_uart_test_echo(IAP_EEPROM_Read_Byte(i)) ;
		if(*p != IAP_EEPROM_Read_Byte(i))
		{
			r=0;
			break;
		}
		p++;
		i++;
	}
	return r;
}
void Function_Login(struct Long_Packet Temp)
{
	if(PasswordCompare(&Temp.TAG[0]))
	{
		int i;
		for(i=0;i<8;i++)
		{
			NetLongPacket.PAC[i]=VAL_MCU_PAC[i];
		}
		NetLongPacket.CMD = STA_LOGIN_OK;
		NetLongPacket.Other[2]= DeviceCount;
		NetLongPacket.DCF = SmartHomeHost_SSC;
		BackNetLongPacket(NetLongPacket);
	}
	else
	{
		NetLongPacket.CMD = STA_LOGIN_ERR;
		NetLongPacket.DCF = SmartHomeHost_SSC;
		BackNetLongPacket(NetLongPacket);
	}
	BUZZER =0 ;
	Delay100ms();Delay100ms();	
	BUZZER = 1;
}
void Function_Logout(struct Long_Packet Temp)
{
	 Temp.CMD = COM_LOGOUT;
	 BackNetLongPacket(Temp);
}
void Net_GetData()	interrupt	8
{
	if(S2CON&S2RI)
	{
		S2CON&=~S2RI;
		NetData[NetData_Num] = S2BUF;
		NetData_Num++;
		if(NetData_Num<6) //处理前六位数据
		{	
			if((NetData_Num==4)&&(NetData[0]==0x01))
			{
				Flag_New_Client = 1;
				NetData_Num = 0;								   	
			}
		}
		else	//处理第六位及以后的数据
		{
			 if((NetData_Num-6)==NetData[4])
			 {
				switch(NetData[4])
				{
					case 0x16:UnionShortPacket(&NetData[7]);break;
					case 0x2F :UnionLongPacket(&NetData[7]);break;
					default : break ;
				 	
				}
				NetData_Num=0;
			 }
		}
	}
	if(S2CON&S2TI)
	{
		S2CON&=~S2TI;
		TXDtemp = 1 ;
	}
}