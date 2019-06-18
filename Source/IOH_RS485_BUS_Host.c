#include <IOH_RS485_BUS_Host.H>
#include <Read_MCU_Info.H>	//包含读取芯片MAC地址函数
#include <STC12C5A60S2.H>
#include <IOSetting.H>		//RS485驱动芯片控制引脚在IOSetting.H文件中设置
#include <intrins.h>
#include <IOHP.H>			//协议共用命令宏定义文件


/*************Extern变量**************/
bit data Flag_RS485_Not_Auto_Register          =0,
	Flag_RS485_Register_Device			=0,
	Flag_Get_RS485_Short_Packet			=0,		
	Flag_Get_RS485_Long_Packet			=0,
	Flag_RS485_BUS_Busy				=0,
	Flag_Request_Send_Host_Address	=0;

struct Long_Packet RS485_LongPacket;
struct Short_Packet RS485_ShortPacket;

/************协议内部变量**************/	
int data	RS485_TimeOut=0;

bit	RS485_BUS_Public_Command=0,While_ACK=0,
	Flag_RS485_Device_ACK			=0,
	Received_Command_Host_Address=0;
char data	RS485_BUS_Addr_Num=0,RS485_BUS_Data_Num=0,
		RS485_BUS_RX_BUF[57],RS485_MAC_Compare_Result=0xAA;

bit Cheek_RS485_BUS(void)  //检测RS485总线是否忙
{
	bit temp = 1 ;
	
	RS485_TimeOut=0;
	if(RXD)
	{
		while(!((!RXD)||(RS485_TimeOut==5)));
		if(RS485_TimeOut==5)
		{
			temp = 0;
		}
	}
	Flag_RS485_BUS_Busy=temp;
	return temp;//返回状态:1为忙,0为空闲	
}
void func_bus_set_status(bit status)
{
	PIN_MAX485_ENABLE= status;//设置RS485总线状态.		

}
void Uart1Init(void)		//57600bps@11.0592MHz
{
	PCON &= 0x7F;		//波特率不倍速
	SCON = 0xF0;		//9位数据,可变波特率,多机通信
	AUXR |= 0x40;		//定时器1时钟为Fosc,即1T
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0F;		//清除定时器1模式位
	TMOD |= 0x20;		//设定定时器1为8位自动重装方式
	TL1 = 0xFA;		//设定定时初值
	TH1 = 0xFA;		//设定定时器重装值
	ET1 = 0;		//禁止定时器1中断
	TR1 = 1;		//启动定时器1
	ES  = 1;	    //允许串口中断
//	MAX485_ENABLE=0;		//485总线允许接收
	func_bus_set_status(COM_SET_BUS_RXD);
}


//void Wait_RS485_Idle(bit RS485_BUS_Busy) //函数等待总线空闲
//{
//	while(RS485_BUS_Busy);	
//}
void RS485_BUS_Free(void)	
{
	int i;
	//MAX485_ENABLE=1;	 //使能MAX485芯片
	func_bus_set_status(COM_SET_BUS_TXD);
	TB8=1;				 //发送地址码
	ES=0;
	for(i=0;i<8;i++)	 //共8位公共地址0x00
	{
		SBUF=0x00;
		while(!TI);
		TI=0;
	}
	TB8=0;				 //发送请求指令
	SBUF=COM_SET_RS485_FREE;
	while(!TI);
	TI=0;
	ES=1;
	RS485_BUS_Data_Num=0;
//	MAX485_ENABLE=0; //从机发送完成,开始接收
func_bus_set_status(COM_SET_BUS_RXD);
	Flag_RS485_BUS_Busy = 0;
}
void Delay1ms()		//@11.0592MHz
{
	unsigned char i, j;

	_nop_();
	i = 11;
	j = 190;
	do
	{
		while (--j);
	} while (--i);
}

void Delay100us()		//@11.0592MHz
{
	unsigned char i, j;

	i = 2;
	j = 15;
	do
	{
		while (--j);
	} while (--i);
}


/***********公布主机地址***********/
void RS485_BUS_Send_Host_Address(bit free)
{
	int i;
	bit r=0;
	while(Flag_RS485_BUS_Busy);
	//Flag_RS485_Get_Host_Address=0;//置标志位为0
//	MAX485_ENABLE=1;	 //使能MAX485芯片
func_bus_set_status(COM_SET_BUS_TXD);
	TB8=1;				 //发送地址码
	for(i=0;i<8;i++)	 //共8位公共地址0x00
	{
		SBUF=0x00;
		while(!TI);
		TI=0;
	}
	TB8=0;				 //发送请求指令
	SBUF=COM_SET_HOST_ADDR;
	while(!TI);
	TI=0;
	for(i=0;i<8;i++)
	{
		SBUF=VAL_MCU_PAC[i];	//用到了读取主机地址函数,需引用"Read_MCU_Info.H"文件
		while(!TI);
		TI=0;
	}
	if(free)
	{
		RS485_BUS_Free();	//释放总线
	}
}
void RS485_BUS_Send_Public_Command(char Command)
{
	int i;
	while(Flag_RS485_BUS_Busy);
	//MAX485_ENABLE=1;	 //使能MAX485芯片
	func_bus_set_status(COM_SET_BUS_TXD);
	TB8=1;				 //发送地址码
	for(i=0;i<8;i++)	 //共8位公共地址0x00
	{
		SBUF=0x00;
		while(!TI);
		TI=0;
	}
	TB8=0;				 //发送请求指令
	SBUF=Command;
	while(!TI);
	TI=0;
	RS485_BUS_Free();	//释放总线
}
bit RS485_BUS_Send_LongPacket(struct Long_Packet temp,bit Free_BUS)
{
	bit result=0;
	int i;
	union{
	struct Long_Packet t;
	char Data[46];
	}combine;
	combine.t=temp;
	
	/********等待总线空闲*********/
	while(Flag_RS485_BUS_Busy);

	func_bus_set_status(COM_SET_BUS_TXD);
	TB8=1;
	RS485_TimeOut=0;
	Flag_RS485_Device_ACK=0;
	While_ACK = 1 ;
	/*********寻呼接收端**********/
	for(i=0;i<8;i++)
	{
		SBUF=temp.PAC[i];
		while(!TI);
		TI=0;
	}
	TB8=0;
	SBUF = TYPE_ACK_REQ;
	while(!TI);
	TI=0;
	for(i=0;i<8;i++)
	{
		SBUF = VAL_MCU_PAC[i];
		while(!TI);
		TI=0;
	}
	//MAX485_ENABLE=0;
	func_bus_set_status(COM_SET_BUS_RXD);
	while(!((Flag_RS485_Device_ACK)||(RS485_TimeOut==5)));//250毫秒超时
	
		/********如果接收端响应寻呼,则发送数据********/
		if(Flag_RS485_Device_ACK)
		{
			//MAX485_ENABLE=1;
			func_bus_set_status(COM_SET_BUS_TXD);
			SBUF=TYPE_POCKET_46;
			while(!TI);
			TI=0;
			for(i=0;i<46;i++)
			{
				SBUF=combine.Data[i];
				while(!TI);
				TI=0;	
			}
			result=1;	//成功获得地址函数返回1，失败返回0。	
		}
	Delay1ms();	
	if(Free_BUS)
		{		
			RS485_BUS_Free();   //释放总线 
		}
	While_ACK = 0;
	//MAX485_ENABLE=0;	//总线状态设为接收
	func_bus_set_status(COM_SET_BUS_RXD);
	return result;
}

/***********************************
函数:	bit RS485_MAC_Compare
用途:	判断输入的是本地地址还是公共地址
输入:	8字节char型数组首地址
输出:	逻辑值Public_MAC_Address	Or	Local_MAC_Address
			详见宏定义
***********************************/
unsigned char CheekAddress(unsigned char *p)
{
	int i;
	unsigned char *temp,r;
	temp = p;
	for(i=0;i<8;i++)
	{
		if( *temp != 0x00)
		{
			break;
		}
		temp++;
	}
	if(i==8)//判断出改地址为公共地址
	{
		 r = PUBLI_PAC_ADDR;
	}
	else	//不是公共地址,继续判断
	{
		temp = p;
		for(i=0;i<8;i++)
		{
			if(*temp != VAL_MCU_PAC[i])
			{
				i=9; //为毛break跳不出来(～￣▽￣)～,也只能这样了
			}
			temp++;	
		}
		if( i==8 )
		{
			r=LOCAL_PAC_ADDR;
		}
		else
		{
			r = OTHER_PAC_ADDR;
		}
	}
	return r;
}
void RS485_ACK(unsigned char *p)
{
	int i;
	TB8=1;	  //如果是本机地址,则发送应答位.
	Delay100us();
	//MAX485_ENABLE=1;	//总线状态设为发送
	PIN_MAX485_ENABLE= COM_SET_BUS_TXD;
	for(i=0;i<8;i++)
	{
		SBUF=*p;
		while(!TI);
		TI=0;
		p++;
	}
	TB8=0;
	SBUF=TYPE_ACK_BACK;
	while(!TI);
	TI=0;
	RS485_BUS_Data_Num	=0;
	//MAX485_ENABLE=0;	//总线状态设为接收
	PIN_MAX485_ENABLE=COM_SET_BUS_RXD;
	Flag_RS485_BUS_Busy=0;
}

void func_uart_test_echo(char buff)
{
	SBUF=buff;
	while(!TI);
	TI=0;
}
void Timer0()	interrupt 1
{
	RS485_TimeOut++;
}
void RS485_BUS() interrupt 4
{
	
	if(RI)
	{
		RI=0;
		Flag_RS485_BUS_Busy=1;
		if(RB8)	 //收到的地址帧 RB8为1
		{
		 	RS485_BUS_RX_BUF[RS485_BUS_Data_Num] = SBUF;
			RS485_BUS_Data_Num++;
			if(RS485_BUS_Data_Num==8)
			{  
				switch(CheekAddress(&RS485_BUS_RX_BUF[0]))
				{
					case  LOCAL_PAC_ADDR:RS485_BUS_Data_Num=0;SM2=0;break;
					case  PUBLI_PAC_ADDR:RS485_BUS_Public_Command=1;SM2=0;break;
					default:RS485_BUS_Data_Num=0;break;
				}
			}
		}
		if(!RB8) //RB8为0,即接受数据帧
		{

			K=!K;
		 	RS485_BUS_RX_BUF[RS485_BUS_Data_Num] = SBUF;
			RS485_BUS_Data_Num++;
			//公共地址及命令处理
			if(RS485_BUS_Public_Command)
			{
				RS485_BUS_Public_Command=0;					 
				switch(RS485_BUS_RX_BUF[8])
				{
					case COM_SET_RS485_FREE	: RS485_BUS_Data_Num = 0;Flag_RS485_BUS_Busy= 0 ;			SM2=1;break;//		释放总线	
					case COM_GET_HOST_ADDR	    : RS485_BUS_Data_Num = 0;Flag_Request_Send_Host_Address=1;	SM2=0;break;	//		返回主机地址
				//	case Command_RS485_BUS_ACK		: RS485_BUS_Data_Num = 0;Flag_RS485_Device_ACK=1; 			SM2=1;break;		//		接收端响应ACK
					default : break;	   			
				}	
			}
			else
			{
				if(RS485_BUS_RX_BUF[0]==TYPE_ACK_REQ)
				{
					if(RS485_BUS_Data_Num==9)
					{
					RS485_BUS_Data_Num=0;
					
					RS485_ACK(&RS485_BUS_RX_BUF[1]);
					SM2=0;
					}
				}
				if(RS485_BUS_RX_BUF[0]==TYPE_ACK_BACK)
				{
			   		RS485_BUS_Data_Num=0;
					Flag_RS485_Device_ACK=1;
					SM2=1;
				}
			/************收到一个长数据包****************/
				if(RS485_BUS_Data_Num==47)
				{
					if(RS485_BUS_RX_BUF[0]== TYPE_POCKET_46)
					{
						//整合数据包
						union{
							char Temp[46];
							struct Long_Packet Received;
						}temp;
						int i;
						for(i=0;i<46;i++)
						{
							temp.Temp[i] =  RS485_BUS_RX_BUF[i+1];
						}
						RS485_LongPacket = temp.Received;
						if(RS485_LongPacket.CMD==COM_CON_DEV_REGIST)
						{
							Flag_RS485_BUS_Busy=0;
						}
						SM2 = 1 ;		//接收完毕
						RS485_BUS_Data_Num = 0;
						Flag_RS485_BUS_Busy=0;
						Flag_Get_RS485_Long_Packet = 1;
					}			 
				}
			}
		}
	}
}