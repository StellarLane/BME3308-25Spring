/*
 * main.c
 */
#include <msp430f6638.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "dr_tft.h"
#include "dr_i2c.h"

#define KB_ADDR 0x21
#define KB_IN 0
#define KB_OUT 1
#define KB_DIR 3

#define SEND_BUF_SIZE 32
#define RECV_BUF_SIZE 32

unsigned char flag0 = 0, flag1 = 0;
unsigned char recv_data[RECV_BUF_SIZE] = {0}; // ���ջ�����
unsigned char send_buffer[SEND_BUF_SIZE] = {0}; // ���ͻ�����
int send_index = 0; // ���ͻ���������
int kb_in; // ���̲�ѯ���

uint16_t cur_kb_input = 0xFFFF, last_kb_input = 0xFFFF;
uint8_t kb_line = 0;

const int KEYBOARD_VALUE[16] = {
    15, 14, 13, 12, 11, 10, 0, 9,
    8, 7, 6, 5, 4, 3, 2, 1
};

const char KB_CHAR_MAP[16] = {
    '#', '*', 'D', 'C', // ��0
    'B', 'A', '0', '9', // ��1
    '8', '7', '6', '5', // ��2
    '4', '3', '2', '1'  // ��3
};

void UART_RS232_Init(void);
void TimerA_Init(void);
void Timer2_Init(void);
void Keyboard_Init(void);
void ProcessKeyPress(int key_index);
void initClock(void);

void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;

    _DINT();
    initClock();
    initI2C();
    UART_RS232_Init();
    init_TFT();
    etft_AreaSet(0, 0, 319, 239, 0);
    TimerA_Init();
    Timer2_Init();
    Keyboard_Init();

    _EINT();

    // �����µ���ʾ����
    etft_DisplayString("Input: ", 80, 60, 65535, 0);
    etft_DisplayString("Send Data: ", 80, 100, 65535, 0);
    etft_DisplayString("Recv Data: ", 80, 140, 65535, 0);

    while(1) {
        if(flag1) {
            flag1 = 0;
            etft_DisplayString(recv_data, 170, 140, 65535, 0);
        }
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{
    static unsigned char i = 0;
    i++;
    if(i >= 20) {
        i = 0;
        flag0 = 0; // ����ʹ���Զ����ͣ�������־λ
    }
}

#pragma vector = TIMER2_A0_VECTOR
__interrupt void Keyboard_Scan_ISR(void)
{
    static int kbcount = 0;
    kbcount++;

    // ��ȡ����״̬
    I2C_CheckQuery(kb_in); // �ն�ȡ����ʹI2Cģ���������

    if(kbcount > 10) {
        kbcount = 0;

        // ���µ�ǰ����״̬
        cur_kb_input &= ~(0xF << (kb_line * 4));
        cur_kb_input |= ((I2C_CheckQuery(kb_in) & 0xF) << (kb_line * 4));

        // ��ⰴ�����£��½��أ�
        uint16_t key_press = (last_kb_input ^ cur_kb_input) & last_kb_input;
        int i = 0;
        for(i = 0; i < 16; i++) {
            if(key_press & (1 << i)) {
                ProcessKeyPress(i);
            }
        }

        last_kb_input = cur_kb_input;

        // �л�����һ��ɨ��
        kb_line = (kb_line + 1) % 4;
        I2C_RequestSend(KB_ADDR, KB_OUT, ~(1 << (kb_line + 4)));
    }
}

void ProcessKeyPress(int key_index)
{
    if(key_index < 0 || key_index > 15) return;

    char key_char = KB_CHAR_MAP[key_index];

    // ���ͼ�����
    if(key_char == '#') {
        if(send_index > 0) {
            // ���������ַ���
            int j = 0;
        	for(j = 0; j < send_index; j++) {
                while (!(UCA1IFG & UCTXIFG)); // �ȴ����ͻ���������
                UCA1TXBUF = send_buffer[j];
            }
            // ��ʾ���͵�����
            etft_DisplayString(send_buffer, 170, 100, 65535, 0);
            send_index = 0;
            memset(send_buffer, 0, SEND_BUF_SIZE);
            // ���������ʾ
            etft_DisplayString("          ", 170, 60, 65535, 0);
        }
    }
    // �˸������
    else if(key_char == '*') {
        if(send_index > 0) {
            send_buffer[--send_index] = '\0';
            // ����������ʾ
            etft_DisplayString(send_buffer, 170, 60, 65535, 0);
        }
    }
    // ��ͨ�ַ�����
    else if(send_index < SEND_BUF_SIZE - 1) {
        send_buffer[send_index++] = key_char;
        send_buffer[send_index] = '\0'; // �����ַ�������
        // ʵʱ��ʾ��ǰ����
        etft_DisplayString(send_buffer, 170, 60, 65535, 0);
    }
}

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
	static int recv_index=0;
    switch(__even_in_range(UCA1IV, 4)) {
        case 0: break; // ���ж�
        case 2:{ // �����ж�
            char received = UCA1RXBUF;

            char debug_buf[2];
			debug_buf[0] = received;
			debug_buf[1] = '\0';
			etft_DisplayString(debug_buf, 250, 180, 65535, 0);

            // �������з��򻺳�����
            if(received == '\r' || received == '\n' || recv_index >= RECV_BUF_SIZE - 1) {
            	if(recv_index > 0) { // ȷ�������ݲ���ʾ
					recv_data[recv_index] = '\0';
					flag1 = 1;
				}
            	recv_index = 0;

            }

            // �洢��Ч�ַ�
            else if(received >= 32 && received <= 126) { // ֻ���ܿɴ�ӡ�ַ�
				recv_data[recv_index++] = received;
				// ʵʱ��ʾ��ǰ����
				recv_data[recv_index] = '\0'; // ȷ���ַ�����ֹ
				etft_DisplayString(recv_data, 170, 140, 65535, 0);
			}
            break;
        }
        case 4: break; // �����ж�
        default: break;
    }
}

void Keyboard_Init(void)
{
    // ���þ�����̵�IO��չ���˿ڷ���
    I2C_RequestSend(KB_ADDR, KB_DIR, 0x0F);
    // ���þ�����̵ĳ�ʼɨ����
    I2C_RequestSend(KB_ADDR, KB_OUT, ~(1 << (kb_line + 4)));
    // Ҫ���Զ���ѯ��������
    kb_in = I2C_AddRegQuery(KB_ADDR, KB_IN);
}

void Timer2_Init(void)
{
    TA2CTL = TASSEL__SMCLK + MC__UP + ID__1;
    TA2CCR0 = (4000000 / 1000) - 1; // 1ms�ж�
    TA2CCTL0 = CCIE;
}

void initClock(void)
{
    while(BAKCTL & LOCKIO) // Unlock XT1 pins for operation
        BAKCTL &= ~(LOCKIO);
    UCSCTL6 &= ~XT1OFF; //����XT1
    P7SEL |= BIT2 + BIT3; //XT2���Ź���ѡ��
    UCSCTL6 &= ~XT2OFF; //����XT2
    while (SFRIFG1 & OFIFG) { //�ȴ�XT1��XT2��DCO�ȶ�
        UCSCTL7 &= ~(DCOFFG+XT1LFOFFG+XT2OFFG);
        SFRIFG1 &= ~OFIFG;
    }

    UCSCTL4 = SELA__XT1CLK + SELS__XT2CLK + SELM__XT2CLK; //����DCO�������ܷ�

    UCSCTL1 = DCORSEL_5; //6000kHz~23.7MHz
    UCSCTL2 = 16000000 / (4000000 / 16); //XT2Ƶ�ʽϸߣ���Ƶ����Ϊ��׼�ɻ�ø��ߵľ���
    UCSCTL3 = SELREF__XT2CLK + FLLREFDIV__16; //XT2����16��Ƶ����Ϊ��׼
    while (SFRIFG1 & OFIFG) { //�ȴ�XT1��XT2��DCO�ȶ�
        UCSCTL7 &= ~(DCOFFG+XT1LFOFFG+XT2OFFG);
        SFRIFG1 &= ~OFIFG;
    }
    UCSCTL5 = DIVA__1 + DIVS__1 + DIVM__1; //�趨����CLK�ķ�Ƶ
    UCSCTL4 = SELA__XT1CLK + SELS__XT2CLK + SELM__DCOCLK; //�趨����CLK��ʱ��Դ
}

// UART��ʼ���������ֲ���
void UART_RS232_Init(void)
{
    P3DIR |= (1<<4)|(1<<5);
    P4DIR |= (1<<4)|(1<<5);
    P4OUT |= (1<<4);
    P4OUT &= ~(1<<5);
    P3OUT |= (1<<5);
    P3OUT &= ~(1<<4);
    P8SEL |= 0x0c;
    UCA1CTL1 |= UCSWRST;
    UCA1CTL1 |= UCSSEL_1;
    UCA1BR0 = 0x03;
    UCA1BR1 = 0x00;
    UCA1MCTL = UCBRS_3+UCBRF_0;
    UCA1CTL1 &= ~UCSWRST;
    UCA1IE |= UCRXIE;
}

// ��ʱ��A��ʼ�����ֲ���
void TimerA_Init(void)
{
    TA0CTL |= MC_1 + TASSEL_2 + TACLR;
    TA0CCTL0 = CCIE;
    TA0CCR0 = 50000;
}
