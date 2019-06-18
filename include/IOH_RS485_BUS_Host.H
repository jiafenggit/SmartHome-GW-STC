#ifndef	__IOH_RS485_BUS_Host_H__
#define	__IOH_RS485_BUS_Host_H__
#include <Data.H>
	extern int data RS485_TimeOut;
	extern bit Flag_RS485_Not_Auto_Register,
		       Flag_RS485_Register_Device,						
				Flag_Get_RS485_Short_Packet,		
				Flag_Get_RS485_Long_Packet,
				Flag_Request_Send_Host_Address,	
				Flag_RS485_BUS_Busy;			

	extern struct Long_Packet RS485_LongPacket;
	extern struct Short_Packet RS485_ShortPacket;

	void Timer0Init();
	void Uart1Init();
	bit Cheek_RS485_BUS();
	void RS485_BUS_Free(void);
	void Wait_RS485_Idle(bit RS485_BUS_Busy);
	void RS485_BUS_Send_Host_Address(bit free) ;
	void RS485_BUS_Send_Public_Command(char Command);
	bit RS485_BUS_Send_LongPacket(struct Long_Packet temp,bit Free_BUS);

	void func_uart_test_echo(char buff);

#endif