#ifndef __Data_H_
#define __Data_H_

struct Long_Packet{		
	char PAC[8]; 
	char CMD;
	int  DCF; 
	char BCF;
	char TAG[19];
	char STA[10];
	int SAC;
	char Other[3];
	};
struct Short_Packet{
	char MAC[8];
	char Command;
	char State[10];
	int SaveAddr;
	};

#endif