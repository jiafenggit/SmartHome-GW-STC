#ifndef	_NetWork_H_
#define	_NetWork_H_
#include <IOH_RS485_BUS_Host.H>
//sbit BUZZER = P2^7;
	#define		S2RI	0x01
	#define		S2TI	0x02
	#define		S2RB8	0x04
	#define		S2TB8	0x08
	#define TypeLongPacket	0x01
	#define	TypeShortPacket	0x02
//	struct Long_Packet
//	{		
//		char MAC[8]; 
//		char Command;
//		int  Device_Class; 
//		char Bus_Class;
//		char Name[19];
//		char State[10];
//		int SaveAddr;
//		char Other[3];
//	};
	extern struct Long_Packet NetLongPacket;

//	struct Short_Packet
//	{
//		char MAC[8];
//		char Command;
//		char State[10];
//		int SaveAddr;
//	};
	extern struct Short_Packet NetShortPacket;
	extern bit Flag_Get_Net_Short_Packet,
				Flag_Get_Net_Long_Packet,
				Flag_New_Client;
	void BackNetShortPacket(struct Short_Packet Temp);
	void BackNetLongPacket(struct Long_Packet Temp);
	void Function_Login(struct Long_Packet Temp);
	void Function_Logout(struct Long_Packet Temp);
	void Uart2Init(void);
#endif