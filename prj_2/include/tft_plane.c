#include <stdint.h>
#include <msp430f6638.h>
#include "tft_base.h"
#include "helper.h"

int g = 1;
extern int plane_x;
extern int plane_vx;
extern int plane_life;
extern int current_game_state;

void init_plane() {
    plane_x = 120;
    plane_vx = 0;
    int i;
    for (i = 0; i < 10; i++) {
        etft_DisplayString("Plane", TFT_YSIZE / 4, plane_x, etft_Color(0, 255, 0), etft_Color(0, 0, 0)); // Clear previous plane position
        __delay_cycles(MCLK_FREQ / 20); // Delay for 100ms
        etft_DisplayString("     ", TFT_YSIZE / 4, plane_x, etft_Color(0, 255, 0), etft_Color(0, 0, 0)); // Clear previous plane position
        __delay_cycles(MCLK_FREQ / 20); // Delay for 100ms
    }
    etft_DisplayString("Plane", TFT_YSIZE / 4, plane_x, etft_Color(255, 0, 0), etft_Color(0, 0, 0));
    return;
}

void check_plane_crash(int upper, int lower) {
    // Check if the plane is within the bounds of the screen
    if (plane_x < lower || plane_x > upper) {
        plane_life--;
        if (plane_life <= 0) {
            current_game_state = 2; // Game over state
        } else {
            // Reset plane position and velocity
            init_plane();
        }
    }
}

void update_plane() {
    // Clear previous plane position
    etft_DisplayString("     ", TFT_YSIZE / 4, plane_x, etft_Color(255, 0, 0), etft_Color(0, 0, 0));

    plane_x = plane_x + (plane_vx + g) / 2;
    plane_vx += g; // Update velocity with gravity
    if (plane_vx > 10 * g) plane_vx = 10 * g; // Limit max velocity

    if (plane_x < 0) {
        plane_x = 0; // Prevent going off the left edge
    } else if (plane_x > TFT_XSIZE - 16) {
        plane_x = TFT_XSIZE - 16; // Prevent going off the right edge
    }
    check_plane_crash(220, 20);
    // Display plane at new position
    etft_DisplayString("Plane", TFT_YSIZE / 4, plane_x, etft_Color(255, 0, 0), etft_Color(0, 0, 0));
}



