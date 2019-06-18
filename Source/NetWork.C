/***************NetWork********************
			���ܼҾӿ���ϵͳ
			����������������
			   2015-02-12 
			   Author:�����
			   E-mail:tianjinkun@kftjk.cn
			   www.ihomediy.cn
Note:
	��ʱ�޷�֧��˫��ͨ��,���������ݰ�ʱ��ע��
	���ݰ����ʱ��!!!
	�������ݰ�����:
		void BackNetLongPacket(struct Long_Packet Temp)
			����43�ֽ��豸��Ϣ��ϸ���ݰ�
			���а���{
			char MAC[8];  	//8�ֽ��豸������ַ
			char Command;	//1�ֽ����ݰ���������
			int  Device_Class; 	   //2�ֽ��豸����
			char Bus_Class;		//1�ֽ���������
			char Name[19];		//19�ֽ��豸����
			char State[10];		//10�ֽ��豸״̬
			int SaveAddr;		//2�ֽ�Flash�豸״̬�洢��ַ
			char Other[3];		//3�ֽ�������Ϣ
			}
		void BackNetShortPacket(struct Short_Packet Temp)
			����12�ֽ��豸������Ϣ���ݰ�
			���а���{
			char MAC[8];  //8�ֽ��豸������ַ
			char Command; //1�ֽ����ݰ���������
			char State[10];//10�ֽ��豸״̬
			int SaveAddr;  //2�ֽ�Flash�豸״̬�洢��ַ
			}
	�յ�һ��43�ֽ��豸��Ϣ��ϸ���ݰ���Flag_Get_Net_Long_Packet
	��־λ��1,����������;���ݽ�����ȫ�ֱ���NetLongPacket��.

	�յ�һ��12�ֽ��豸������Ϣ���ݰ���Flag_Get_Net_Short_Packet
	��־λ��1,����������;���ݽ�����ȫ�ֱ���NetShortPacket��.

	*����ģ����ͨ������2����MCU,����Ҫ��������豸�ĳ�ʼ��
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
	AUXR &= 0xF7;		//�����ʲ�����
	S2CON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x04;		//���������ʷ�����ʱ��ΪFosc,��1T
	BRT = 0xDC;		//�趨���������ʷ�������װֵ
	AUXR |= 0x10;		//�������������ʷ�����
	IE2= 0X01;			//��������2�ж�
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
    while(!TXDtemp);  //��S2TI=0���ڴ˵ȴ�
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
		if(NetData_Num<6) //����ǰ��λ����
		{	
			if((NetData_Num==4)&&(NetData[0]==0x01))
			{
				Flag_New_Client = 1;
				NetData_Num = 0;								   	
			}
		}
		else	//��������λ���Ժ������
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