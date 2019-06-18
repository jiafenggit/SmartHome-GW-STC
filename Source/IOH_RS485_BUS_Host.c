#include <IOH_RS485_BUS_Host.H>
#include <Read_MCU_Info.H>	//������ȡоƬMAC��ַ����
#include <STC12C5A60S2.H>
#include <IOSetting.H>		//RS485����оƬ����������IOSetting.H�ļ�������
#include <intrins.h>
#include <IOHP.H>			//Э�鹲������궨���ļ�


/*************Extern����**************/
bit data Flag_RS485_Not_Auto_Register          =0,
	Flag_RS485_Register_Device			=0,
	Flag_Get_RS485_Short_Packet			=0,		
	Flag_Get_RS485_Long_Packet			=0,
	Flag_RS485_BUS_Busy				=0,
	Flag_Request_Send_Host_Address	=0;

struct Long_Packet RS485_LongPacket;
struct Short_Packet RS485_ShortPacket;

/************Э���ڲ�����**************/	
int data	RS485_TimeOut=0;

bit	RS485_BUS_Public_Command=0,While_ACK=0,
	Flag_RS485_Device_ACK			=0,
	Received_Command_Host_Address=0;
char data	RS485_BUS_Addr_Num=0,RS485_BUS_Data_Num=0,
		RS485_BUS_RX_BUF[57],RS485_MAC_Compare_Result=0xAA;

bit Cheek_RS485_BUS(void)  //���RS485�����Ƿ�æ
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
	return temp;//����״̬:1Ϊæ,0Ϊ����	
}
void func_bus_set_status(bit status)
{
	PIN_MAX485_ENABLE= status;//����RS485����״̬.		

}
void Uart1Init(void)		//57600bps@11.0592MHz
{
	PCON &= 0x7F;		//�����ʲ�����
	SCON = 0xF0;		//9λ����,�ɱ䲨����,���ͨ��
	AUXR |= 0x40;		//��ʱ��1ʱ��ΪFosc,��1T
	AUXR &= 0xFE;		//����1ѡ��ʱ��1Ϊ�����ʷ�����
	TMOD &= 0x0F;		//�����ʱ��1ģʽλ
	TMOD |= 0x20;		//�趨��ʱ��1Ϊ8λ�Զ���װ��ʽ
	TL1 = 0xFA;		//�趨��ʱ��ֵ
	TH1 = 0xFA;		//�趨��ʱ����װֵ
	ET1 = 0;		//��ֹ��ʱ��1�ж�
	TR1 = 1;		//������ʱ��1
	ES  = 1;	    //�������ж�
//	MAX485_ENABLE=0;		//485�����������
	func_bus_set_status(COM_SET_BUS_RXD);
}


//void Wait_RS485_Idle(bit RS485_BUS_Busy) //�����ȴ����߿���
//{
//	while(RS485_BUS_Busy);	
//}
void RS485_BUS_Free(void)	
{
	int i;
	//MAX485_ENABLE=1;	 //ʹ��MAX485оƬ
	func_bus_set_status(COM_SET_BUS_TXD);
	TB8=1;				 //���͵�ַ��
	ES=0;
	for(i=0;i<8;i++)	 //��8λ������ַ0x00
	{
		SBUF=0x00;
		while(!TI);
		TI=0;
	}
	TB8=0;				 //��������ָ��
	SBUF=COM_SET_RS485_FREE;
	while(!TI);
	TI=0;
	ES=1;
	RS485_BUS_Data_Num=0;
//	MAX485_ENABLE=0; //�ӻ��������,��ʼ����
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


/***********����������ַ***********/
void RS485_BUS_Send_Host_Address(bit free)
{
	int i;
	bit r=0;
	while(Flag_RS485_BUS_Busy);
	//Flag_RS485_Get_Host_Address=0;//�ñ�־λΪ0
//	MAX485_ENABLE=1;	 //ʹ��MAX485оƬ
func_bus_set_status(COM_SET_BUS_TXD);
	TB8=1;				 //���͵�ַ��
	for(i=0;i<8;i++)	 //��8λ������ַ0x00
	{
		SBUF=0x00;
		while(!TI);
		TI=0;
	}
	TB8=0;				 //��������ָ��
	SBUF=COM_SET_HOST_ADDR;
	while(!TI);
	TI=0;
	for(i=0;i<8;i++)
	{
		SBUF=VAL_MCU_PAC[i];	//�õ��˶�ȡ������ַ����,������"Read_MCU_Info.H"�ļ�
		while(!TI);
		TI=0;
	}
	if(free)
	{
		RS485_BUS_Free();	//�ͷ�����
	}
}
void RS485_BUS_Send_Public_Command(char Command)
{
	int i;
	while(Flag_RS485_BUS_Busy);
	//MAX485_ENABLE=1;	 //ʹ��MAX485оƬ
	func_bus_set_status(COM_SET_BUS_TXD);
	TB8=1;				 //���͵�ַ��
	for(i=0;i<8;i++)	 //��8λ������ַ0x00
	{
		SBUF=0x00;
		while(!TI);
		TI=0;
	}
	TB8=0;				 //��������ָ��
	SBUF=Command;
	while(!TI);
	TI=0;
	RS485_BUS_Free();	//�ͷ�����
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
	
	/********�ȴ����߿���*********/
	while(Flag_RS485_BUS_Busy);

	func_bus_set_status(COM_SET_BUS_TXD);
	TB8=1;
	RS485_TimeOut=0;
	Flag_RS485_Device_ACK=0;
	While_ACK = 1 ;
	/*********Ѱ�����ն�**********/
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
	while(!((Flag_RS485_Device_ACK)||(RS485_TimeOut==5)));//250���볬ʱ
	
		/********������ն���ӦѰ��,��������********/
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
			result=1;	//�ɹ���õ�ַ��������1��ʧ�ܷ���0��	
		}
	Delay1ms();	
	if(Free_BUS)
		{		
			RS485_BUS_Free();   //�ͷ����� 
		}
	While_ACK = 0;
	//MAX485_ENABLE=0;	//����״̬��Ϊ����
	func_bus_set_status(COM_SET_BUS_RXD);
	return result;
}

/***********************************
����:	bit RS485_MAC_Compare
��;:	�ж�������Ǳ��ص�ַ���ǹ�����ַ
����:	8�ֽ�char�������׵�ַ
���:	�߼�ֵPublic_MAC_Address	Or	Local_MAC_Address
			����궨��
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
	if(i==8)//�жϳ��ĵ�ַΪ������ַ
	{
		 r = PUBLI_PAC_ADDR;
	}
	else	//���ǹ�����ַ,�����ж�
	{
		temp = p;
		for(i=0;i<8;i++)
		{
			if(*temp != VAL_MCU_PAC[i])
			{
				i=9; //Ϊëbreak��������(��������)��,Ҳֻ��������
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
	TB8=1;	  //����Ǳ�����ַ,����Ӧ��λ.
	Delay100us();
	//MAX485_ENABLE=1;	//����״̬��Ϊ����
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
	//MAX485_ENABLE=0;	//����״̬��Ϊ����
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
		if(RB8)	 //�յ��ĵ�ַ֡ RB8Ϊ1
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
		if(!RB8) //RB8Ϊ0,����������֡
		{

			K=!K;
		 	RS485_BUS_RX_BUF[RS485_BUS_Data_Num] = SBUF;
			RS485_BUS_Data_Num++;
			//������ַ�������
			if(RS485_BUS_Public_Command)
			{
				RS485_BUS_Public_Command=0;					 
				switch(RS485_BUS_RX_BUF[8])
				{
					case COM_SET_RS485_FREE	: RS485_BUS_Data_Num = 0;Flag_RS485_BUS_Busy= 0 ;			SM2=1;break;//		�ͷ�����	
					case COM_GET_HOST_ADDR	    : RS485_BUS_Data_Num = 0;Flag_Request_Send_Host_Address=1;	SM2=0;break;	//		����������ַ
				//	case Command_RS485_BUS_ACK		: RS485_BUS_Data_Num = 0;Flag_RS485_Device_ACK=1; 			SM2=1;break;		//		���ն���ӦACK
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
			/************�յ�һ�������ݰ�****************/
				if(RS485_BUS_Data_Num==47)
				{
					if(RS485_BUS_RX_BUF[0]== TYPE_POCKET_46)
					{
						//�������ݰ�
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
						SM2 = 1 ;		//�������
						RS485_BUS_Data_Num = 0;
						Flag_RS485_BUS_Busy=0;
						Flag_Get_RS485_Long_Packet = 1;
					}			 
				}
			}
		}
	}
}