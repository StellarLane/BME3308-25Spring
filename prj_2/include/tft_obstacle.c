#include <stdint.h>
#include <msp430f6638.h>
#include "tft_base.h"
#include "helper.h"

extern Obstacle obstacles[10];
extern int current_game_level;
extern int current_score;

void update_obstacles() {
    int i;
    for (i = 0; i < 10; i++)
    {
        if (obstacles[i].active) {
            // Clear previous obstacle position
            etft_AreaSet(obstacles[i].x, obstacles[i].y, obstacles[i].x + obstacles[i].width - 1, obstacles[i].y + obstacles[i].height - 1, etft_Color(0, 0, 0));
            
            // Update obstacle position
            obstacles[i].x -= obstacles[i].v * current_game_level; // Speed increases with level
            
            // Check if obstacle is out of bounds
            if (obstacles[i].x < -obstacles[i].width) {
                // Reset obstacle position
                obstacles[i].x = TFT_XSIZE;
                obstacles[i].y = rand() % (TFT_YSIZE - 20); // Random y position
                obstacles[i].active = 0; // Deactivate obstacle
                current_score += 1; // Increase score when obstacle is passed
            }
            
            // Draw obstacle at new position
            etft_AreaSet(obstacles[i].x, obstacles[i].y, obstacles[i].x + obstacles[i].width - 1, obstacles[i].y + obstacles[i].height - 1, etft_Color(255, 0, 0));
        }
    }
}

void initiate_obstacles() {
    int i;
    for (i = 0; i < 10; i++) {
        initiate_obstacle(&obstacles[i]); // Initialize each obstacle
    }
}

void initiate_obstacle(Obstacle* ob) {
    ob->x = 300; // Set initial x position
    ob->y = rand() % (TFT_YSIZE - 20); // Random y position
    ob->width = 20; // Width of obstacle
    ob->height = 10; // Height of obstacle
    ob->v = 2; // Speed of obstacle
    ob->active = 0; // Set obstacle as inactive
    // etft_AreaSet(ob->x, ob->y, ob->x + ob->width - 1, ob->y + ob->height - 1, etft_Color(255, 0, 0)); // Draw obstacle
}

void try_launch_obstacle() {
    int i;
    if (rand() % 10 > current_game_level) {
        return;
    }
    for (i = 0; i < 10; i++) {
        if (!obstacles[i].active) { // Find an inactive obstacle
            obstacles[i].active = 1; // Activate the obstacle
            break;
        }
    }
}