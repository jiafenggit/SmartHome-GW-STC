#include <STC12C5A60S2.H>
#include <TimeAndDelay.H>
#include <IOSetting.H>
#include <AT24C16.H>

#include <FlashOperate.H>

void start() //启动（I方C）总线
{
	SDA=1; 
	Delay2us(); 
	SCL=1; 
	Delay2us();
	SDA=0; 
	Delay2us();
	SCL=0; 
	Delay2us();
}
void stop() //停止（I方C）总线
{
	SDA=0; 
	Delay2us(); 
	SCL=1; 
	Delay2us(); 
	SDA=1; 
	Delay2us();
}
void IICwrite(unsigned char y) //写一个字节
{
unsigned char x,temp;
	temp=y;
	for (x=0;x<8;x++)
	{
		temp=temp<<1; 
		SCL=0; 
		Delay2us(); 
		SDA=CY; 
		Delay2us(); 
		SCL=1; 
		Delay2us();
	}
	SCL=0; 
	Delay2us(); 
	SDA=1; 
	Delay2us();
}

unsigned char IICread() //读一个字节
{
	unsigned char w,v,u=0;
	SCL=0; 
	Delay2us(); 
	SDA=1;
	for (w=0;w<8;w++)
	{
		Delay2us(); 
		SCL=1; 
		Delay2us();
		if (SDA==1) v=1;
		else v=0;
		u=(u<<1)|v;
		SCL=0;
	}
	Delay2us(); 
	return(u);
}
void ACK() //(I方C）应答位
	{
	unsigned char t=0;
	SCL=1; 
	Delay2us();
	while ((SDA==1)&&(t<255))t++;
	SCL=0; 
	Delay2us();
}

/********************************************************************
                AT24C16 初始化子程序
*********************************************************************/
void Init_24C16() //
{
	SCL=1;
	Delay2us();
	SDA=1;                    
	Delay2us();
}
/********************************************************************
                从AT24C16 的address地址中读出一字节数据
*********************************************************************/
unsigned char Read_Byte_24C16(unsigned int address)
{
	unsigned char s;
	union{
		unsigned int a;
		unsigned char b[2];
		}exchange;
	unsigned char device;
	exchange.a=address;	  //1xa0+page address
	device = (exchange.b[0]&0x07);
	device=device<<1;
	device |= 0xa0;
	start(); 
	IICwrite(device);
	ACK(); 
	IICwrite(exchange.b[1]);
	ACK();
	device |= 0x01; 
	start();
	IICwrite(device); 
	ACK();
	s=IICread();
	stop();
	Delay2us();
	Delay2us();
	return(s);
}
/********************************************************************
                向AT24C16 的address地址中写入一字节数据
*********************************************************************/
void Write_Byte_24C16(unsigned int address,unsigned char info)
{
	
	union{
		unsigned int a;
		unsigned char b[2];
		}exchange;
	unsigned char device;

//	echoint(address) ;

	exchange.a=address;	  //0xa0+page address
	device = (exchange.b[0]);
	device=device<<1;
	device |= 0xa0;
	start(); 
	IICwrite(device);
	ACK(); 
	IICwrite(exchange.b[1]);
	ACK(); 
	IICwrite(info);
	ACK(); 
	stop();
	Delay2ms();//2毫秒延时,不能再快了!!
}
void Write_Block_24C16(unsigned int address,unsigned char* info,unsigned char length)
{

/***块写暂时没有成功,用字写+延时实现****/
//	union{
//		unsigned int a;
//		unsigned char b[2];
//		}exchange;
//	unsigned char device,i;
//	exchange.a=address;	  //0xa0+page address
//	device = (exchange.b[0]&0x07);
//	device=device<<1;
//	device |= 0xa0;
//	start(); 
//	IICwrite(device);
//	ACK(); 
//	IICwrite(exchange.b[1]);
//	ACK(); 
//	for(i=0;i<length;i++)
//	{
//		IICwrite(*info);
//		ACK();
//		info++; 
//	}	
//	stop();
//	Delay2ms();//2毫秒延时,不能再快了!!
	unsigned char i;
	for(i=0;i<length;i++)
	{
		Write_Byte_24C16(address+i,*info);
		info++;
	}
}