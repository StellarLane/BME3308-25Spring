/*
 * main.c
 */
#include <msp430f6638.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "dr_lcdseg.h"   //调用段式液晶驱动头文件

#define XT2_FREQ   4000000

#define MCLK_FREQ 16000000
#define SMCLK_FREQ 4000000



void initClock()
{
  while(BAKCTL & LOCKIO) BAKCTL &= ~(LOCKIO);
  UCSCTL6 &= ~XT1OFF; //启动XT1，选择内部时钟源
  P7SEL |= BIT2 + BIT3; //XT2引脚功能选择
  UCSCTL6 &= ~XT2OFF; //启动XT2
  while (SFRIFG1 & OFIFG) //等待XT1、XT2与DCO稳定
  {
    UCSCTL7 &= ~(DCOFFG+XT1LFOFFG+XT2OFFG);
    SFRIFG1 &= ~OFIFG;
  }
  UCSCTL4 = SELA__XT1CLK + SELS__XT2CLK + SELM__XT2CLK; //避免DCO调整中跑飞
  UCSCTL1 = DCORSEL_5; //6000kHz~23.7MHz
  UCSCTL2 = MCLK_FREQ / (XT2_FREQ / 16); //XT2频率较高，分频后作为基准可获得更高的精度
  UCSCTL3 = SELREF__XT2CLK + FLLREFDIV__16; //XT2进行16分频后作为基准
  while (SFRIFG1 & OFIFG) //等待XT1、XT2与DCO稳定
  {
    UCSCTL7 &= ~(DCOFFG+XT1LFOFFG+XT2OFFG);
    SFRIFG1 &= ~OFIFG;
  }
  UCSCTL5 = DIVA__1 + DIVS__1 + DIVM__1; //设定几个CLK的分频
  UCSCTL4 = SELA__XT1CLK + SELS__XT2CLK + SELM__DCOCLK; //设定几个CLK的时钟源
}

void main(void)
{
	unsigned char num1;
  int i;
  int32_t num2;
  WDTCTL = WDTPW | WDTHOLD; // 停止看门狗
  initClock();              // 配置系统时钟
  initLcdSeg();             // 初始化段式液晶
  const int date[6] = {0, 3, 0, 5, 2, 5};
  while (1) // 进入程序主循环
  {
    for (i = 0; i < 6; i++)
    {
      LCDSEG_SetDigit(i, date[i]);
      LCDSEG_SetSpecSymbol(4);
    }
    __delay_cycles(MCLK_FREQ * 4);
    for (i = 0; i < 6; i++)
    {
      LCDSEG_SetDigit(i, -1);
    }
    for (i = 0; i < 3; i++)
    {
      display_SJTU(i);
      __delay_cycles(MCLK_FREQ * 4);
    }

  }
}


