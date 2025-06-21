#include <stdint.h>
#include <stdio.h>
#include <msp430f6638.h>
#include "tft_base.h"

extern int high_score;
extern int plane_life;
extern int current_game_state;
extern int plane_x;
extern int plane_vx;
extern int current_score;

void init_clock()
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

void init_timerA(void) {
    TA0CTL |= MC_1 + TASSEL_2 + TACLR; // 选择SMCLK为时钟源，设置为增计数模式，清零计数器
    TA0CCTL0 = CCIE;                   // 使能CCR0的中断
    TA0CCR0  = 50000;                  // 设置计数上限为50000，到达时产生中断
}

// init button and led
void init_GIPO() {
    P4DIR |= BIT5 | BIT6 | BIT7; // Set P4.5, P4.6, P4.7 as output (LEDs)
    P4OUT &= ~(BIT5 | BIT6 | BIT7); // Turn off LEDs initially
    P4DIR &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4); 
    P4REN |= BIT0 | BIT1 | BIT2 | BIT3 | BIT4; // Enable pull-up/pull-down resistors on P4.0~P4.4
    P4OUT |= BIT0 | BIT1 | BIT2 | BIT3 | BIT4; // Set pull-up resistors on P4.0~P4.4
    P4IE |= BIT0 | BIT1 | BIT2 | BIT3 | BIT4;  // Enable interrupts for P4.0~P4.4
    P4IES |= BIT0 | BIT1 | BIT2 | BIT3 | BIT4; // Set interrupt on high-to-low transition for P4.0~P4.4
    P4IFG &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4); // Clear interrupt flags for P4.0~P4.4
    P4OUT |= BIT4; // Set pull-up resistor on P4.4
    P4IE |= BIT4; // Enable interrupt for P4.4
    P4IES |= BIT4; // Set interrupt on high-to-low transition
    P4IFG &= ~BIT4; // Clear interrupt flag for P4.4
    P5DIR |= BIT7; // Set P5.7 as output (LED)
    P5OUT &= ~BIT7; // Turn off P5.7 LED initially
    P8DIR |= BIT0; // Set P8.0 as output (LED)
    P8OUT &= ~BIT0; // Turn off P8.0 LED initially
}

void update_led(int plane_life) {
    switch (plane_life) {
    case 5:
        P4OUT |= (BIT5 | BIT6 | BIT7);
        P5OUT |= BIT7; 
        P8OUT |= BIT0; 
        break;
    case 4:
        P4OUT |= (BIT6 | BIT7);
        P4OUT &= ~BIT5; // Turn off LED 1
        P5OUT |= BIT7;
        P8OUT |= BIT0;
        break;
    case 3:
        P4OUT |= BIT7;
        P4OUT &= ~(BIT5 | BIT6); // Turn off LED 1 and LED 2
        P5OUT |= BIT7;
        P8OUT |= BIT0;
        break;
    case 2:
        P4OUT &= ~(BIT5 | BIT6 | BIT7); // Turn off all LEDs
        P5OUT |= BIT7; // Keep LED 2 on
        P8OUT |= BIT0; // Keep LED 3 on
        break;
    case 1:
        P4OUT &= ~(BIT5 | BIT6 | BIT7); // Turn off all LEDs
        P5OUT &= ~BIT7; // Turn off LED 2
        P8OUT |= BIT0; 
        break;
    case 0:
        P4OUT &= ~(BIT5 | BIT6 | BIT7); // Turn off all LEDs
        P5OUT &= ~BIT7; // Turn off LED 2
        P8OUT &= ~BIT0; // Turn off LED 3
        break;
    default:
        break;
    }
}

void rules() {
    etft_DisplayString("Game Rules:", 10, 10, etft_Color(255, 255, 255), etft_Color(0, 0, 0)); // Display game rules
    etft_DisplayString("1. Avoid obstacles", 10, 30, etft_Color(255, 255, 255), etft_Color(0, 0, 0)); 
    etft_DisplayString("2. Use S6 to go down", 10, 50, etft_Color(255, 255, 255), etft_Color(0, 0, 0)); // Display instruction
    etft_DisplayString("3. Use S5 to go up", 10, 70, etft_Color(255, 255, 255), etft_Color(0, 0, 0)); // Display instruction
    etft_DisplayString("   and S4 to go even faster", 10, 90, etft_Color(255, 255, 255), etft_Color(0, 0, 0)); // Display instruction
    etft_DisplayString("4. Press any key to return", 10, 110, etft_Color(255, 255, 255), etft_Color(0, 0, 0)); // Display instruction
}

void welcome_message() {
    char high_score_str[20];
    sprintf(high_score_str, "High Score %d", high_score); // Format high score string
    etft_DisplayString("Welcome to the Little Plane!", 10, 10, etft_Color(255, 255, 255), etft_Color(0, 0, 0)); // Display welcome message
    etft_DisplayString("Press S3 to start", 10, 30, etft_Color(255, 255, 255), etft_Color(0, 0, 0)); // Display instruction
    etft_DisplayString("Press S5 to see the rules", 10, 50, etft_Color(255, 255, 255), etft_Color(0, 0, 0)); // Display instruction
    etft_DisplayString(high_score_str, 10, 70, etft_Color(255, 255, 255), etft_Color(0, 0, 0)); // Display high score
}

void game_over() {
    update_led(0);
    char score_str[20];
    char high_score_str[20];
    if (current_score > high_score) {
        sprintf(high_score_str, "New High Score!");
        high_score = current_score; // Update high score if current score is higher
    } else {
        sprintf(high_score_str, "High Score: %d", high_score); // Format high score string
    }
    sprintf(score_str, "Score: %d", current_score); // Format score string
    etft_DisplayString("Game Over!", 10, 10, etft_Color(255, 0, 0), etft_Color(0, 0, 0)); // Display game over message
    etft_DisplayString(score_str, 10, 30, etft_Color(255, 0, 0), etft_Color(0, 0, 0)); // Display score
    etft_DisplayString(high_score_str, 10, 50, etft_Color(255, 0, 0), etft_Color(0, 0, 0)); // Display high score
    etft_DisplayString("Press any key to return to welcome", 10, 70, etft_Color(255, 0, 0), etft_Color(0, 0, 0)); // Display instruction
}

void button_handler(unsigned int pin) {
    switch (current_game_state)
    {
    case -1:
        etft_AreaSet(0, 0, TFT_XSIZE - 1, TFT_YSIZE - 1, 0x0000); // Clear screen
        current_game_state = 0; // Return to welcome state
        break;
    case 0:
        switch (pin)
        {
        case 3:
            current_game_state = 1; // Start game
            break;
        case 5:
            etft_AreaSet(0, 0, TFT_XSIZE - 1, TFT_YSIZE - 1, 0x0000); // Clear screen
            current_game_state = -1; // Show rules
            break;
        default:
            break;
        }
        break;
    case 1:
        switch (pin)
        {
        case 4:
            plane_vx -= 2; // Increase velocity
            break;
        case 5:
            plane_vx -= 1; // Increase velocity
            break;
        case 6:
            plane_vx += 1; // Decrease velocity
            break;
        case 7:
            current_game_state = 2;
            break;
        default:
            break;
        }
        break;
    case 2:
        plane_life = 5;
        current_game_state = 0;
        break;
    default:
        break;
    }
}

int get_adc_noise() {
    ADC12CTL0 = ADC12SHT0_2 | ADC12ON;      // 打开ADC
    ADC12CTL1 = ADC12SHP;                   // 使用采样保持脉冲模式
    ADC12MCTL0 = ADC12INCH_5;               // 选择A5通道（假设A5悬空）
    ADC12CTL0 |= ADC12ENC | ADC12SC;        // 启动转换
    while (ADC12CTL1 & ADC12BUSY);          // 等待转换完成
    return ADC12MEM0;                       // 返回采样值
}
