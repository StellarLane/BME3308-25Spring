#ifndef __DR_TFT_H_
#define __DR_TFT_H_

#include <stdint.h>

#ifndef MCLK_FREQ
  #define MCLK_FREQ 20000000
#endif

#ifndef SMCLK_FREQ
  #define SMCLK_FREQ 20000000
#endif

#define SPI_FREQ 10000000

#define TFT_XSIZE 240
#define TFT_YSIZE 320

#define TFTREG_RAM_XADDR  0x0201
#define TFTREG_RAM_YADDR  0x0200
#define TFTREG_RAM_ACCESS 0x0202

#define TFTREG_SOFT_RESET 0x0003

#define TFTREG_WIN_MINX   0x0212
#define TFTREG_WIN_MAXX   0x0213
#define TFTREG_WIN_MINY   0x0210
#define TFTREG_WIN_MAXY   0x0211



/* TFT���ײ�ӿ� */

//��ʼ��TFT
void init_TFT();

//��TFT������һ����ַ�������Ƿ��ͳɹ�
int tft_SendIndex(uint16_t val);

//��TFT������һ�����ݣ������Ƿ��ͳɹ�
int tft_SendData(uint16_t val);

//��TFT���ļĴ���reg��������data�������Ƿ��ͳɹ�
int tft_SendCmd(uint16_t reg, uint16_t data);

/* TFT���߲�ӿ� */
/* ���и߲�ӿ�����X��Y�Ե������ӿڴ�XΪ��YΪ�� */

//��0~255��ʾ��RGB��ɫת��ΪTFT��Ļʹ�õ���ɫ
static inline uint16_t etft_Color(uint8_t r, uint8_t g, uint8_t b)
{
  uint16_t temp = 0;
  temp |= (r << 8) & 0xF800;
  temp |= (g << 3) & 0x07E0;
  temp |= (b >> 3) & 0x001F;
  return temp;
}

//��һ��������Ϊĳ����ɫ
void etft_AreaSet(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY, uint16_t color);

//��ָ����λ����ʾһ���ַ���
void etft_DisplayString(const char* str, uint16_t sx, uint16_t sy, uint16_t fRGB, uint16_t bRGB);

//��ָ����λ����ʾһ��ͼƬ��image��24λλͼ��������ʾ
//������˳������ҡ����µ���(����˳��ת)��ÿ3�ֽ�һ�����أ�˳��ΪB��G��R��ÿ���ֽ�����0������4��������
//�Գ���24λλͼ����0x36���Ƶ��ļ�ĩβ����
void etft_DisplayImage(const uint8_t* image, uint16_t sx, uint16_t sy, uint16_t width, uint16_t height);

#endif
