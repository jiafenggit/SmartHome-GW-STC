#include <STC12C5A60S2.H>
#include <TimeAndDelay.H>
#include <intrins.H>

void Timer0Init(void)		//50����@11.0592MHz
{
	AUXR &= 0x7F;		//��ʱ��ʱ��12Tģʽ
	TMOD &= 0xF0;		//���ö�ʱ��ģʽ
	TMOD |= 0x01;		//���ö�ʱ��ģʽ
	TL0 = 0x00;		//���ö�ʱ��ֵ
	TH0 = 0x4C;		//���ö�ʱ��ֵ
	TF0 = 0;		//���TF0��־
	TR0 = 1;		//��ʱ��0��ʼ��ʱ
	ET0 =1;
}

void Delay2us()   //��ʱ10us
{
	unsigned char i;

	_nop_();
	_nop_();
	i = 2;
	while (--i);
}
void Delay2ms()		//@11.0592MHz
{
	unsigned char i, j;

	i = 22;
	j = 128;
	do
	{
		while (--j);
	} while (--i);
}
void Delay100ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	i = 5;
	j = 52;
	k = 195;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}