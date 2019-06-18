#include<STC12C5A60S2.H>
/**AT24C16_Pin_Setting**/
sbit SCL		=P3^5;
sbit SDA		=P3^6;

/**MAX485_ENABLE_Pin_Setting**/
sbit PIN_MAX485_ENABLE=P3^4;

/**Buzzer_Pin_Setting**/
sbit Buzzer		=P2^7;
sbit K = P2^4;

/**SPI_BUS_Pin_Setting**/
sbit MISO		=P1^6;
sbit MOSI		=P1^5;
sbit SCK	    =P1^7;
sbit CE	    	=P1^1;
sbit CSN		=P1^4;
sbit IRQ		=P3^2;

/**ReSet_Key_Pin_Setting**/
sbit ReSet_Key	=P2^0;

/**Status_LED_Pin_Setting**/
sbit LED1		=P2^6;	//双色LED灯绿色Pin
sbit LED2		=P2^5;	//双色LED灯红色Pin

sbit	RXD = P3^0;