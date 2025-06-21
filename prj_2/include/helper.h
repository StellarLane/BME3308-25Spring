// clock initiation
void init_clock();

// GIPO
void init_GIPO();

// show plane's life through LED
void update_led(int life);

void init_TimerA();

// set the area of [startX, startY] to [endX, endY] with the specified color
void etft_AreaSet(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY, uint16_t color);

// display a string at the specified position with foreground and background colors
void etft_DisplayString(const char* str, uint16_t sx, uint16_t sy, uint16_t fRGB, uint16_t bRGB);

//initialize the plane
void init_plane();

// update the plane position
void update_plane();

// check if the plane crashes
void check_plane_crash(int upper, int lower);

typedef struct {
    int x;
    int y;
    int width;
    int height;
    int v;
    int active;
} Obstacle;

void initiate_obstacles();

void update_obstacles();

void initiate_obstacle();

void try_lauch_obstacle();