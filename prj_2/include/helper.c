#include <stdint.h>
#include <stdio.h>
#include <msp430f6638.h>
#include "tft_base.h"
#include "helper.h"
#include "dr_lcdseg.h"

extern int high_score;
extern int plane_life;
extern int current_game_state;
extern int current_game_level;
extern int plane_x;
extern int plane_vx;
extern int current_score;
extern Obstacle obstacles[10];
extern Boss boss;
extern Obstacle boss_obstacles[5];
extern Obstacle boss_skill_obstacles[5];
extern int laser;

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
    TA0CTL |= MC_1 + TASSEL_2 + TACLR; // 閫夋嫨SMCLK涓烘椂閽熸簮锛岃缃负澧炶鏁版ā寮忥紝娓呴浂璁℃暟鍣�
    TA0CCTL0 = CCIE;                   // 浣胯兘CCR0鐨勪腑鏂�
    TA0CCR0  = 50000;                  // 璁剧疆璁℃暟涓婇檺涓�0000锛屽埌杈炬椂浜х敓涓柇
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

void boss_defeat() {
    etft_AreaSet(0, 0, TFT_XSIZE - 1, TFT_YSIZE - 1, etft_Color(0, 0, 0)); // Clear screen
    etft_DisplayString("Boss Defeated!", 10, 10, etft_Color(0, 255, 0), etft_Color(0, 0, 0)); // Display boss defeat message
    while (1)
        ;
}

void launch_laser() {
    if (current_game_state != 3) {
        return; // Only launch laser when the game is in boss state
    }
    if (!boss.active) {
        return; // Boss not active, no need to launch laser
    }
    // Launch laser from plane position as a revenge skill
    etft_AreaSet(80, plane_x + 8, boss.x + boss.width - 1, plane_x + 13, etft_Color(0, 255, 255)); // Draw laser from plane to boss
    // Check if the laser hits the boss
    if (plane_x + 8 <= boss.y + boss.width && plane_x + 13 >= boss.y) {
        boss.hp--;
        if (boss.hp <= 0) {
            boss.active = 0;
            boss_defeat();                                                                                       // Return to normal game state or handle boss defeat
            etft_AreaSet(boss.x, boss.y, boss.x + boss.width - 1, boss.y + boss.width - 1, etft_Color(0, 0, 0)); // Clear boss
        }
    }
    __delay_cycles(MCLK_FREQ / 1000); // Delay for laser effect
    etft_AreaSet(80, plane_x + 8, boss.x + boss.width - 1, plane_x + 13, etft_Color(0, 0, 0)); // Clear laser area
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
        case 7:
            etft_AreaSet(0, 0, TFT_XSIZE - 1, TFT_YSIZE - 1, 0x0000); // Clear screen
            current_game_state = 3; // Game over state
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
            // current_game_state = 2;
            break;
        default:
            break;
        }
        break;
    case 2:
        plane_life = 5;
        current_game_state = 0;
        break;
    case 3:
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
            // current_game_state = 2;
            break;
        case 3:
            if (laser == 0) break;
            laser--;
            launch_laser();
        default:
            break;
        }
        break;
    default:
        break;
    }
}

int get_adc_noise() {
    ADC12CTL0 = ADC12SHT0_2 | ADC12ON;      // 鎵撳紑ADC
    ADC12CTL1 = ADC12SHP;                   // 浣跨敤閲囨牱淇濇寔鑴夊啿妯″紡
    ADC12MCTL0 = ADC12INCH_5;               // 閫夋嫨A5閫氶亾锛堝亣璁続5鎮┖锛�
    ADC12CTL0 |= ADC12ENC | ADC12SC;        // 鍚姩杞崲
    while (ADC12CTL1 & ADC12BUSY);          // 绛夊緟杞崲瀹屾垚
    return ADC12MEM0;                       // 杩斿洖閲囨牱鍊�
}

void show_status() {
    char status_str[50] = "";
    sprintf(status_str, "Plane v: %d, Score: %d ", plane_vx, current_score);
    etft_DisplayString(status_str, 10, 10, etft_Color(255, 255, 255), etft_Color(0, 0, 0)); // Display status
}

void show_boss_status() {
    char boss_status_str[50] = "";
    sprintf(boss_status_str, "Boss HP: %d, Laser: %d ", boss.hp, laser);
    etft_DisplayString(boss_status_str, 10, 10, etft_Color(255, 0, 0), etft_Color(0, 0, 0)); // Display boss status
}

void clear_obstacles() {
    int i;
    for (i = 0; i < 10; i++) {
        if (obstacles[i].active &&
            obstacles[i].x <= 150) {
                obstacles[i].active = 0; // Deactivate obstacle
                etft_AreaSet(obstacles[i].x, obstacles[i].y, obstacles[i].x + obstacles[i].width - 1, obstacles[i].y + obstacles[i].height - 1, etft_Color(0, 0, 0)); // Clear obstacle area
                obstacles[i].x = TFT_XSIZE; // Reset obstacle position
                obstacles[i].y = 30 + rand() % (TFT_YSIZE - 50); // Random y position
                obstacles[i].width = 20; // Width of obstacle
                obstacles[i].height = 10; // Height of obstacle
                obstacles[i].v = 2; // Speed of obstacle
            }
    }
}

void clear_boss_obstacles() {
    int i;
    for (i = 0; i < 5; i++) {
        if (boss_obstacles[i].active &&
            boss_obstacles[i].x <= 150) {
                boss_obstacles[i].active = 0; // Deactivate obstacle
                etft_AreaSet(boss_obstacles[i].x, boss_obstacles[i].y, boss_obstacles[i].x + boss_obstacles[i].width - 1, boss_obstacles[i].y + boss_obstacles[i].height - 1, etft_Color(0, 0, 0)); // Clear obstacle area
                boss_obstacles[i].x = TFT_XSIZE; // Reset obstacle position
                boss_obstacles[i].y = 30 + rand() % (TFT_YSIZE - 50); // Random y position
                boss_obstacles[i].width = 20; // Width of obstacle
                boss_obstacles[i].height = 10; // Height of obstacle
                boss_obstacles[i].v = 2; // Speed of obstacle
            }
    }
}

void clear_boss_skill_obstacles() {
    int i;
    for (i = 0; i < 5; i++) {
        if (boss_skill_obstacles[i].active &&
            boss_skill_obstacles[i].x <= 150) {
                boss_skill_obstacles[i].active = 0; // Deactivate obstacle
                etft_AreaSet(boss_skill_obstacles[i].x, boss_skill_obstacles[i].y, boss_skill_obstacles[i].x + boss_skill_obstacles[i].width - 1, boss_skill_obstacles[i].y + boss_skill_obstacles[i].height - 1, etft_Color(0, 0, 0)); // Clear obstacle area
                boss_skill_obstacles[i].x = TFT_XSIZE; // Reset obstacle position
                boss_skill_obstacles[i].y = 30 + rand() % (TFT_YSIZE - 50); // Random y position
                boss_skill_obstacles[i].width = 20; // Width of obstacle
                boss_skill_obstacles[i].height = 10; // Height of obstacle
                boss_skill_obstacles[i].v = 2; // Speed of obstacle
            }
    }
}

void init_boss() {
    if (boss.active) {
        return; // Boss already active, no need to reinitialize
    }
    // Initialize boss position and attributes
    boss.x = 250; // Set initial x position
    boss.y = 100; // Set initial y position
    boss.width = 50; // Width of boss
    boss.hp = 3; // Initial health points of boss
    boss.active = 1; // Set boss as active
    // Display boss on the screen
    etft_AreaSet(boss.x, boss.y, boss.x + boss.width - 1, boss.y + boss.width - 1, etft_Color(255, 0, 0)); // Draw boss area
}

void update_boss() {
    if (!boss.active) {
        return; // Boss not active, no need to update
    }
    // Clear previous boss position
    etft_AreaSet(boss.x, boss.y, boss.x + boss.width - 1, boss.y + boss.width - 1, etft_Color(0, 0, 0));
    
    // Randomly move boss to somewhere near its current position
    int dx = (rand() % 11) - 5; // Random value between -5 and 5
    int dy = (rand() % 11) - 5; // Random value between -5 and 5

    boss.x += dx * current_game_level / 2;
    boss.y += dy * current_game_level / 2;

    // Keep boss within screen bounds
    if (boss.x < 200) boss.x = 200;
    if (boss.x > TFT_XSIZE - boss.width) boss.x = TFT_XSIZE - boss.width;
    if (boss.y < 0) boss.y = 0;
    if (boss.y > TFT_YSIZE - boss.width) boss.y = TFT_YSIZE - boss.width;
    
    // Draw boss at new position
    etft_AreaSet(boss.x, boss.y, boss.x + boss.width - 1, boss.y + boss.width - 1, etft_Color(255, 0, 0)); // Draw boss area
}

void try_launch_boss_obstacle() {
    if (current_game_state != 3) {
        return; // Only launch boss obstacles when the game is in boss state
    }
    if (!boss.active) {
        return; // Boss not active, no need to launch obstacles
    }
    // Randomly launch a boss obstacle
    if (1) { // Adjust probability based on game level
        int i;
        for (i = 0; i < 5; i++) {
            if (!boss_obstacles[i].active) { // Find an inactive obstacle
                boss_obstacles[i].active = 1; // Activate the obstacle
                boss_obstacles[i].x = boss.x + boss.width; // Set obstacle position near boss
                boss_obstacles[i].y = 50 + rand() % (TFT_YSIZE - 70); // Random y position
                boss_obstacles[i].width = 20; // Width of obstacle
                boss_obstacles[i].height = 10; // Height of obstacle
                boss_obstacles[i].v = 1;       // Speed of obstacle, can be adjusted
                break;
            }
        }
    }
}

void update_boss_obstacles() {
    int i;
    for (i = 0; i < 5; i++) {
        if (boss_obstacles[i].active) {
            // Clear previous obstacle position
            etft_AreaSet(boss_obstacles[i].x, boss_obstacles[i].y, boss_obstacles[i].x + boss_obstacles[i].width - 1, boss_obstacles[i].y + boss_obstacles[i].height - 1, etft_Color(0, 0, 0));
            
            // Update obstacle position
            boss_obstacles[i].x -= boss_obstacles[i].v * current_game_level; // Speed increases with level
            
            // Check if obstacle is out of bounds
            if (boss_obstacles[i].x < -boss_obstacles[i].width) {
                // Reset obstacle position
                boss_obstacles[i].active = 0; // Deactivate obstacle
                continue; // Skip to next iteration
            }
            
            // Draw obstacle at new position
            etft_AreaSet(boss_obstacles[i].x, boss_obstacles[i].y, boss_obstacles[i].x + boss_obstacles[i].width - 1, boss_obstacles[i].y + boss_obstacles[i].height - 1, etft_Color(255, 0, 0)); // Draw obstacle area
        }
    }
}

void try_launch_boss_skill_obstacle() {
    if (current_game_state != 3) {
        return; // Only launch boss skill obstacles when the game is in boss state
    }
    if (boss_skill_obstacles[0].active) {
        return; // Boss skill obstacles already active, no need to launch
    }
    if (!boss.active) {
        return; // Boss not active, no need to launch skill obstacles
    }
    // if (rand() % 10 < 2) { // Adjust probability based on game level
    //     return; // Random chance to not launch skill obstacles
    // }
    // Activate all boss skill obstacles at once
    int i;
    for (i = 0; i < 3; i++) {
        boss_skill_obstacles[i].active = 1;
        boss_skill_obstacles[i].x = boss.x + boss.width; // Set obstacle position near boss
        boss_skill_obstacles[i].y = boss.y + i * 20; // Random y position
        boss_skill_obstacles[i].width = 20; // Width of obstacle
        boss_skill_obstacles[i].height = 10; // Height of obstacle
        boss_skill_obstacles[i].v = 1; // Speed of obstacle, can be adjusted
    }
}

void update_boss_skill_obstacles() {
    int i;
    for (i = 0; i < 5; i++) {
        if (boss_skill_obstacles[i].x > 0 || boss_skill_obstacles[i].x < boss.x) {
            ; // Skip if obstacle is not in the active range
        } else {
            boss_skill_obstacles[i].active = 1;
        }
        if (boss_skill_obstacles[i].active) {
            // Clear previous obstacle position
            etft_AreaSet(boss_skill_obstacles[i].x, boss_skill_obstacles[i].y, boss_skill_obstacles[i].x + boss_skill_obstacles[i].width - 1, boss_skill_obstacles[i].y + boss_skill_obstacles[i].height - 1, etft_Color(0, 0, 0));
            
            // Update obstacle position
            boss_skill_obstacles[i].x -= boss_skill_obstacles[i].v * current_game_level; // Speed increases with level
            
            // Check if obstacle is out of bounds
            if (boss_skill_obstacles[i].x < 0) {
                // Reset obstacle position
                boss_skill_obstacles[i].active = 0; // Deactivate obstacle
                continue; // Skip to next iteration
            }
            
            // Draw obstacle at new position
            etft_AreaSet(boss_skill_obstacles[i].x, boss_skill_obstacles[i].y, boss_skill_obstacles[i].x + boss_skill_obstacles[i].width - 1, boss_skill_obstacles[i].y + boss_skill_obstacles[i].height - 1, etft_Color(255, 0, 0)); // Draw obstacle area
        }
    }
}
