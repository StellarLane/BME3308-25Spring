/*
 * main.c
 */

#include <msp430f6638.h>
#include <stdint.h>
#include <stdio.h>
#include "dr_tft.h"

unsigned char flag0=0,flag1=0;
unsigned char send_data[]={'0','\0'};
unsigned char recv_data[]={'0','\0'};


void UART_RS232_Init(void);	//RS232�ӿڳ�ʼ��
void TimerA_Init(void);		//��ʱ��TA��ʼ������

void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;//�رտ��Ź�

	UART_RS232_Init();		//��ʼ��RS232�ӿ�
	init_TFT();				//��ʼ��TFT��Ļ
	etft_AreaSet(0,0,319,239,0);	//TFT����
	TimerA_Init();					//��ʼ����ʱ��
	_EINT();				//�����ж�

	etft_DisplayString("Send Data: ",80,100,65535,0);	//TFT����ʾ�������ݵı�ʶ
	etft_DisplayString("Recv Data: ",80,140,65535,0);	//TFT����ʾ�������ݵı�ʶ

	while(1)
	{
		if(flag0)			//flag0�ڶ�ʱ����ʱ����1sʱ����ֵΪ1
		{
			flag0=0;
			UCA1TXBUF=send_data[0];		//дһ���ַ������ͻ��淢������
			etft_DisplayString(send_data,170,100,65535,0);	//TFT������ʾ���͵��ַ�
			send_data[0]++;				//�ַ���1
			if(send_data[0]>'9')		//�ַ�����'9'��������'0'
				send_data[0]='0';
		}

		if(flag1)			//flag1�ڽ����ж��б���ֵΪ1
		{
			flag1=0;
			etft_DisplayString(recv_data,170,140,65535,0);	//TFT��Ļ����ʾ���յ����ַ�
		}
	}
}


#pragma vector=USCI_A1_VECTOR	//USCI�жϷ�����
__interrupt void USCI_A1_ISR(void)
{
	switch(__even_in_range(UCA1IV,4))
	{
	case 0:break;			//���ж�
	case 2:					//�����жϴ���
		while(!(UCA1IFG&UCTXIFG));	//�ȴ���ɽ���
		recv_data[0]=UCA1RXBUF;		//���ݶ���
		flag1=1;					//�ñ�ʶ���ݵ�ֵ
		break;
	case 4:break;			//�����жϲ�����
	default:break;			//��������޲���

	}
}

#pragma vector = TIMER0_A0_VECTOR	//��ʱ��TA�жϷ�����
__interrupt void Timer_A (void)
{
	static unsigned char i=0;
	i++;
	if(i>=20)				//������ʮ��Ϊ1s
	{
		i=0;
		flag0=1;			//�ı��ʶ���ݵ�ֵ
	}
}


void UART_RS232_Init(void)	//RS232�ӿڳ�ʼ������
{
	/*ͨ����P3.4��P3.5��P4.4��P4.5������ʵ��ͨ��ѡ��
	 	 ʹUSCI�л���UARTģʽ*/
	P3DIR|=(1<<4)|(1<<5);
	P4DIR|=(1<<4)|(1<<5);
	P4OUT|=(1<<4);
	P4OUT&=~(1<<5);
	P3OUT|=(1<<5);
	P3OUT&=~(1<<4);
	P8SEL|=0x0c;	//ģ�鹦�ܽӿ����ã���P8.2��P8.3��ΪUSCI�Ľ��տ��뷢���
	UCA1CTL1|=UCSWRST;	//��λUSCI
	UCA1CTL1|=UCSSEL_1;	//���ø���ʱ�ӣ����ڷ����ض�������
	UCA1BR0=0x03;		//���ò�����
	UCA1BR1=0x00;
	UCA1MCTL=UCBRS_3+UCBRF_0;
	UCA1CTL1&=~UCSWRST;	//������λ
	UCA1IE|=UCRXIE;		//ʹ�ܽ����ж�
}

void TimerA_Init(void)		//��ʱ��TA��ʼ������
{
	TA0CTL |= MC_1 + TASSEL_2 + TACLR;	//ʱ��ΪSMCLK,�Ƚ�ģʽ����ʼʱ���������
	TA0CCTL0 = CCIE;					//�Ƚ����ж�ʹ��
	TA0CCR0  = 50000;					//�Ƚ�ֵ��Ϊ50000���൱��50ms��ʱ����
}
