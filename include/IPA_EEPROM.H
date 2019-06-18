#ifndef	_IPA_EEPROM_H_
#define	_IPA_EEPROM_H_

#define CMD_IDLE	0
#define CMD_READ	1
#define	CMD_PROGRAM	2
#define	CMD_ERASE		3
#define	ENABLE_IAP	0x83
#define IAP_ADDRESS	0x0000

void IAP_EEPROM_Erase_Sector(int addr);
char IAP_EEPROM_Read_Byte(int addr);
void IAP_EEPROM_Write_Byte(int addr,char dat);
void IAP_EEPROM_Write_Strings(int addr,char *s);


#endif