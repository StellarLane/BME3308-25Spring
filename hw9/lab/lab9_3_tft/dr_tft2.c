#include <msp430.h>
#include "dr_tft.h"
#include "dr_tft_ascii.h"

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

void etft_DisplayImage(const uint8_t* image, uint16_t sx, uint16_t sy, uint16_t width, uint16_t height)
{
  uint16_t i,j;
  uint32_t row_length = width * 3; //每行像素数乘3
  if(row_length & 0x3) //非4整倍数
  {
    row_length |= 0x03;
    row_length += 1;
  }
  const uint8_t *ptr = image + (height - 1) * row_length;
  tft_SendCmd(TFTREG_WIN_MINX, sx);
  tft_SendCmd(TFTREG_WIN_MINY, sy);
  tft_SendCmd(TFTREG_WIN_MAXX, sx + width - 1);
  tft_SendCmd(TFTREG_WIN_MAXY, sy + height - 1);

  tft_SendCmd(TFTREG_RAM_XADDR, sx);
  tft_SendCmd(TFTREG_RAM_YADDR, sy);

  tft_SendIndex(TFTREG_RAM_ACCESS);
  for(i=0;i<height;i++)
  {
    for(j=0;j<width;j++)
    {
      tft_SendData(etft_Color(ptr[2], ptr[1], ptr[0]));
      ptr += 3;
    }
    ptr -= width * 3 + row_length;
  }
}



// 显示16x16汉字 (通用版本)
void etft_DisplayChinese16x16(uint16_t sx, uint16_t sy, const unsigned char *bitmap,
                             int char_count, uint16_t fRGB, uint16_t bRGB) {
    uint16_t cx, cy;
    int bytes_per_row = char_count * 2;  // 每行字节数 = 汉字数量 × 2

    // 临时缓冲区存储单个汉字点阵
    unsigned char char_bitmap[32];  // 32字节/汉字
    int char_idx = 0;
    for(char_idx = 0; char_idx < char_count; char_idx++) {
        // 从源数据中提取单个汉字点阵
    	int row = 0;
        for(row = 0; row < 16; row++) {
            // 计算源数据位置：行索引 × 每行总字节数 + 汉字索引 × 2
            int src_pos = row * bytes_per_row + char_idx * 2;

            // 复制该行的2个字节
            char_bitmap[row * 2] = bitmap[src_pos];
            char_bitmap[row * 2 + 1] = bitmap[src_pos + 1];
        }

        // 设置当前汉字的显示区域 (16x16像素)
        uint16_t char_x = sx + char_idx * 16;
        tft_SendCmd(TFTREG_WIN_MINX, char_x);
        tft_SendCmd(TFTREG_WIN_MINY, sy);
        tft_SendCmd(TFTREG_WIN_MAXX, char_x + 15);
        tft_SendCmd(TFTREG_WIN_MAXY, sy + 15);
        tft_SendCmd(TFTREG_RAM_XADDR, char_x);
        tft_SendCmd(TFTREG_RAM_YADDR, sy);
        tft_SendIndex(TFTREG_RAM_ACCESS);

        // 显示当前汉字
        for(cy = 0; cy < 16; cy++) {
            // 每行2字节（16像素）
            uint16_t line = (char_bitmap[cy * 2] << 8) | char_bitmap[cy * 2 + 1];

            // 处理一行中的16个像素
            for(cx = 0; cx < 16; cx++) {
                // 高位在前 (MSB对应左侧像素)
                if(line & 0x8000) {
                    tft_SendData(fRGB);  // 前景色
                } else {
                    tft_SendData(bRGB);  // 背景色
                }
                line <<= 1;  // 左移处理下一位
            }
        }
    }
}
