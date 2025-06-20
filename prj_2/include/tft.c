#include <stdint.h>
#include <msp430f6638.h>
#include "tft_base.h"
#include "tft_ascii.h"
#include "helper.h"

void etft_AreaSet(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY, uint16_t color)
{
  uint16_t i,j;
  tft_SendCmd(TFTREG_WIN_MINX, startX);
  tft_SendCmd(TFTREG_WIN_MINY, startY);
  tft_SendCmd(TFTREG_WIN_MAXX, endX);
  tft_SendCmd(TFTREG_WIN_MAXY, endY);
          
  tft_SendCmd(TFTREG_RAM_XADDR, startX);
  tft_SendCmd(TFTREG_RAM_YADDR, startY);
  
  tft_SendIndex(TFTREG_RAM_ACCESS);
  for(i=0;i<endY - startY + 1;i++)
  {
    for(j=0;j<endX - startX + 1;j++)
    {
      tft_SendData(color);
    }
  }
}

void etft_DisplayString(const char* str, uint16_t sx, uint16_t sy, uint16_t fRGB, uint16_t bRGB)
{
  uint16_t cc = 0;
  uint16_t cx, cy;
  
  while(1)
  {
    char curchar = str[cc];
    if(curchar == '\0') //字符串已发送完
      return;

    cx = 0;
    cy = 0;
    //屏幕是横的，XY要对调
    tft_SendCmd(TFTREG_WIN_MINX, sx);//x start point
    tft_SendCmd(TFTREG_WIN_MINY, sy);//y start point
    tft_SendCmd(TFTREG_WIN_MAXX, sx+7);//x end point
    tft_SendCmd(TFTREG_WIN_MAXY, sy+15);//y end point
    tft_SendCmd(TFTREG_RAM_XADDR, sx);//x start point
    tft_SendCmd(TFTREG_RAM_YADDR, sy);//y start point
    tft_SendIndex(TFTREG_RAM_ACCESS);
    
    uint16_t color;
    while(1)
    {
      if(cx >= 8)
      {
        cx = 0;
        cy++;
        if(cy >= 16)
        { //一个字符发送完毕
          cc++; //下一个字符
          sx+=8;
          if(sx >= TFT_YSIZE) //越过行末
          {
            sx = 0;
            sy += 16;
          }
          break;
        }
      }
      
      if((tft_ascii[curchar*16 + cy] << cx) & 0x80)
        color = fRGB;
      else
        color = bRGB;
      
      tft_SendData(color);
      cx++; //X自增 
    }
  }
}