#include <msp430.h>
#include <msp430f6638.h>
#include <stdint.h>
#include <stdio.h>
#include "dr_tft.h"
#include "cat.h"
#include "name.h"
#include "1.h"
#include "3.h"

void initClock()
{
  while(BAKCTL & LOCKIO)
    BAKCTL &= ~(LOCKIO);
  UCSCTL6 &= ~XT1OFF;
  P7SEL |= BIT2 + BIT3;
  UCSCTL6 &= ~XT2OFF;
  while (SFRIFG1 & OFIFG) {
    UCSCTL7 &= ~(DCOFFG+XT1LFOFFG+XT2OFFG);
    SFRIFG1 &= ~OFIFG;
  }

  UCSCTL4 = SELA__XT1CLK + SELS__XT2CLK + SELM__XT2CLK;

  UCSCTL1 = DCORSEL_5;
  UCSCTL2 = 20000000 / (4000000 / 16);
  UCSCTL3 = SELREF__XT2CLK + FLLREFDIV__16;
  while (SFRIFG1 & OFIFG) {
    UCSCTL7 &= ~(DCOFFG+XT1LFOFFG+XT2OFFG);
    SFRIFG1 &= ~OFIFG;
  }
  UCSCTL5 = DIVA__1 + DIVS__1 + DIVM__1;
  UCSCTL4 = SELA__XT1CLK + SELS__DCOCLK + SELM__DCOCLK;
}

int main( void )
{
  WDTCTL = WDTPW + WDTHOLD;

  _DINT();

  initClock();
  init_TFT();

  _EINT();

  etft_AreaSet(0,0,319,239,0);

  while(1)
  {
    etft_AreaSet(0,0,39,239,0);
    etft_AreaSet(40,0,79,239,31);
    etft_AreaSet(80,0,119,239,2016);
    etft_AreaSet(120,0,159,239,63488);
    etft_AreaSet(160,0,199,239,2047);
    etft_AreaSet(200,0,239,239,63519);
    etft_AreaSet(240,0,279,239,65504);
    etft_AreaSet(280,0,319,239,65535);
    __delay_cycles(MCLK_FREQ*3);

    etft_AreaSet(0,0,319,239,0);
    __delay_cycles(MCLK_FREQ);

    etft_DisplayChinese16x16(50, 50, bitmap_bytes_cs, 2, 0xF800, 0xFFE0);

    etft_DisplayChinese16x16(100, 50, bitmap_bytes_dsc, 3, 0xF800, 0xFFE0);

    etft_DisplayChinese16x16(50, 100, bitmap_bytes_lyl, 3, 0xF800, 0xFFE0);

    etft_DisplayChinese16x16(100, 100, bitmap_bytes_zyh, 3, 0xF800, 0xFFE0);

    __delay_cycles(MCLK_FREQ*3);
    etft_AreaSet(0,0,319,239,0);
    __delay_cycles(MCLK_FREQ);

    etft_DisplayImage(gImage_3, 100, 100, 100, 100);
    __delay_cycles(MCLK_FREQ*3);
    etft_AreaSet(0,0,319,239,0);
    __delay_cycles(MCLK_FREQ);
  }
}
