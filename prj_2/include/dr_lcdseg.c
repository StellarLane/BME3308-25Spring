#include <msp430f6638.h>
#include <msp430.h>
#include <stdint.h>
#include "dr_lcdseg.h"

const uint8_t SEG_CTRL_BIN[17] =
{
  0x3F,
  0x06,
  0x5B,
  0x4F,
  0x66,
  0x6D,
  0x7D,
  0x07,
  0x7F,
  0x6F,
  0x77,
  0x7C,
  0x39,
  0x5E,
  0x79,
  0x71,
  0x40,
};

void initLcdSeg()
{
  P5SEL |= BIT3 + BIT4 + BIT5;
  LCDBPCTL0 = 0x0FFF;
  LCDBCTL0 = LCDDIV_21 + LCDPRE__4 + LCD4MUX;
  LCDBMEMCTL |= LCDCLRM;
  LCDBCTL0 |= LCDSON + LCDON;
}

void LCDSEG_SetDigit(int pos, int value)
{
  if(pos < 0 || pos > 6)
    return;

  uint8_t temp, mem;
  if(value < 0 || value > 16)
    temp = 0x00;
  else
    temp = SEG_CTRL_BIN[value];

  const static uint8_t map[7] ={ BIT7, BIT6, BIT5, BIT0, BIT1, BIT3, BIT2 };

  mem = LCDMEM[pos];
  mem &= 0x10;
  int i;
  for(i=0;i<7;++i)
  {
    if(temp & (1 << i))
      mem |= map[i];
  }
  LCDMEM[pos] = mem;
}

void LCDSEG_SetSpecSymbol(int pos)
{
  LCDMEM[pos] |= 0x10;
}

void LCDSEG_ResetSpecSymbol(int pos)
{
  LCDMEM[pos] &= ~0x10;
}

void LCDSEG_DisplayNumber(int32_t num, int dppos)
{
  int curpos = 0, isneg = 0;

  if(num < 0)
  {
    isneg = 1;
    num = -num;
  }

  while(1)
  {
    int digit = num % 10;
    num /= 10;
    LCDSEG_SetDigit(curpos++, digit);
    if(num == 0)
      break;
  }

  if(isneg)
    LCDSEG_SetDigit(curpos++, 16);

  while(curpos < 6)
    LCDSEG_SetDigit(curpos++, -1);

  int i;
  for(i=3;i<=5;++i)
    LCDSEG_ResetSpecSymbol(i);

  if(dppos > 0 && dppos <= 3)
    LCDSEG_SetSpecSymbol(6 - dppos);
}
