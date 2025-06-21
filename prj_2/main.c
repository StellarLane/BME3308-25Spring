#include <msp430.h> 
#include <msp430f6638.h>
#include <stdio.h>
#include "./include/tft_base.h"
#include "./include/helper.h"

/*
 * main.c
 */

//current game state: -1 rules, 0 welcome, 1 playing, 2 game over
int current_game_state = 0;
int high_score = 0; // High score
int plane_vx = 0; // Plane velocity
int plane_x = 120; // Plane position
int plane_life = 5;
int current_score = 0; // Current score
int current_game_level = 1; // Current game level
Obstacle obstacles[10]; // Array of obstacles

#define clear_screen() etft_AreaSet(0, 0, TFT_YSIZE - 1, TFT_XSIZE - 1, 0x0000); // Clear screen

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    _DINT(); // Disable interrupts
    init_clock();
    init_TFT();
    _EINT(); // Enable interrupts
    init_GIPO(); // Initialize GPIO for buttons and LEDs
    init_timerA(); // Initialize Timer A for periodic interrupts
    clear_screen(); // Clear the screen
    int a;
    while (1) {
        while (current_game_state <= 0) {
            if (current_game_state == 0) welcome_message(); // Display welcome message
            if (current_game_state == -1) rules(); // Display rules message
        }
        clear_screen(); 
        initiate_obstacles(); // Initialize obstacles
        init_plane(); // Initialize plane position and velocity
        current_score = 0;
        current_game_level = 1; // Reset game level
        while (current_game_state == 1) {
            update_led(plane_life); // Update LED display based on plane life
            update_plane(); // Update plane position
            __delay_cycles(MCLK_FREQ / 20); // Delay for 100ms
            update_obstacles(); // Update obstacles
        }
        clear_screen(); // Clear the screen after game over
        while (current_game_state == 2) {
            game_over(); // Display game over message
        }
        clear_screen(); // Clear the screen after game over
    }
    return 0;
}

#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void) {
    unsigned int pin;
    for (pin = 0; pin <= 4; pin++) {
        if (P4IFG & (1 << pin)) {
            break;
        }
    }
    pin = 7 - pin; // button pressed
    P4IFG &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4); // Clear interrupt flags for P4.0~P4.4
    __delay_cycles(MCLK_FREQ / 1000); // Debounce delay
    button_handler(pin);
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void) {
    static int i = 0;
    if (current_game_state != 1)
        return; // Only update during game play
    i++;
    if (i >= 2500) { 
        char level_str[10];
        sprintf(level_str, "Level: %d", current_game_level); // Format level string
        etft_DisplayString(level_str, 10, 10, etft_Color(255, 0, 0), etft_Color(0, 0, 0)); // Display current level
        i = 0; // Reset counter
        current_game_level  = current_game_level >= 10 ? 10 : current_game_level + 1; // Increase game level
    }
    if (i % 250 == 0) {
        try_launch_obstacle();
    }
}