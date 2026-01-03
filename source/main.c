#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>

#define FB_WIDTH  1280
#define FB_HEIGHT 720
#define CELL_SIZE 80

#define GRID_COLS 16
#define GRID_ROWS 9
#define FRAME_RATE 15
#define CELL_BORDER 1

#define COLOR_SNAKE 0xFF00FF00
#define COLOR_FOOD 0xFF0000FF

#define INITIAL_SNAKE_SIZE 3
#define INITIAL_X 8
#define INITIAL_Y 5
#define RANDOM_SEED 333

#define DIR_IDLE 0
#define DIR_UP 1
#define DIR_RIGHT 2
#define DIR_DOWN 3
#define DIR_LEFT 4

#include "text.h"

struct State
{
    u8* segments;
    u8 size;
    u8 direction;
    u8 foodX;
    u8 foodY;
    bool dead;
    bool paused;
    bool quit;
    u32 frame;
    u32 score;
};

void genFood(u8* segments, u8 size, u8* out_x, u8* out_y) {
    u8 foodY;
    u8 foodX;
    bool done = false;
    while (!done) {
        foodY = rand() % GRID_ROWS;
        foodX = rand() % GRID_COLS;
        done = true;
        for (u32 i = 0; i < size; i++) {
            if (foodX == segments[i*2 + 1] && foodY == segments[i*2]) {
                done = false;
                break;
            }
        }
    }
    *out_y = foodY;
    *out_x = foodX;
}

// Initialize state with default values
struct State stateInit() {
    struct State state;
    state.segments = malloc(sizeof(u8) * GRID_COLS * GRID_ROWS * 2);
    state.size = INITIAL_SNAKE_SIZE;
    
    // Initialize snake at starting position
    for (u8 i = 0; i < state.size; i++) {
        state.segments[i*2] = INITIAL_Y;
        state.segments[i*2 + 1] = INITIAL_X;
    }
    
    // Generate initial food position
    genFood(state.segments, state.size, &state.foodX, &state.foodY);
    
    // Initialize game state
    state.dead = false;
    state.paused = false;
    state.quit = false;
    state.direction = DIR_IDLE;
    state.frame = 0;
    state.score = 0;
    
    return state;
}

void checkInput(PadState* pad, struct State* state) {
    padUpdate(pad);
    u64 kDown = padGetButtonsDown(pad);
    
    if (kDown & HidNpadButton_Minus) {
        state->paused = !state->paused;
    }

    if (kDown & HidNpadButton_Plus) {
        state->quit = true;
        return;
    }
    
    if (kDown & HidNpadButton_Up && state->direction != DIR_DOWN)
        state->direction = DIR_UP;
    else if (kDown & HidNpadButton_Right && state->direction != DIR_LEFT)
        state->direction = DIR_RIGHT;
    else if (kDown & HidNpadButton_Down && state->direction != DIR_UP)
        state->direction = DIR_DOWN;
    else if (kDown & HidNpadButton_Left && state->direction != DIR_RIGHT)
        state->direction = DIR_LEFT;
}

void update(struct State* state) {
    if (state->frame != 0 || state->direction == DIR_IDLE) return;
    
    u8 prevY = state->segments[0];
    u8 prevX = state->segments[1];
    
    // Update segments
    for (u32 i = 0; i < state->size; i++) {
        if (i != 0) {
            u8 y = state->segments[i*2];
            u8 x = state->segments[i*2 + 1];
            state->segments[i*2] = prevY;
            state->segments[i*2 + 1] = prevX;
            prevX = x;
            prevY = y;
        } else {
            switch (state->direction) {
                case DIR_UP:
                    state->segments[0] -= 1;
                    break;
                case DIR_RIGHT:
                    state->segments[1] += 1;
                    break;
                case DIR_DOWN:
                    state->segments[0] += 1;
                    break;
                case DIR_LEFT:
                    state->segments[1] -= 1;
                    break;
                default:
                    break;
            }
        }
    }
    
    // Check wall and self collision
    if (state->segments[0] >= GRID_ROWS || state->segments[0] < 0 || state->segments[1] >= GRID_COLS || state->segments[1] < 0) {
        state->dead = true;
        return;
    }
    for (u32 i = 1; i < state->size; i++) {
        if (state->segments[0] == state->segments[i*2] && state->segments[1] == state->segments[i*2 + 1]) {
            state->dead = true;
            return;
        }
    }
    
    // Check food collision
    if (state->segments[0] == state->foodY && state->segments[1] == state->foodX) {
        state->score += 1;
        state->segments[state->size*2] = prevY;
        state->segments[state->size*2 + 1] = prevX;
        state->size += 1;
        genFood(state->segments, state->size, &state->foodX, &state->foodY);
    }
}

void draw(Framebuffer fb, struct State* state) {
    u32 stride;
    u32* framebuf = (u32*) framebufferBegin(&fb, &stride);

    // Clear screen
    for (u32 y = 0; y < FB_HEIGHT; y++) {
        for (u32 x = 0; x < FB_WIDTH; x++) {
            u32 pos = y * stride / sizeof(u32) + x;
            framebuf[pos] = 0;
        }
    }
    
    // Draw segments
    for (u32 i = 0; i < state->size; i++) {
        u32 a = state->segments[i*2] * CELL_SIZE;
        u32 b = state->segments[i*2 + 1] * CELL_SIZE;
        for (u32 y = a + CELL_BORDER; y < a + CELL_SIZE - CELL_BORDER; y ++) {
            for (u32 x = b + CELL_BORDER; x < b + CELL_SIZE - CELL_BORDER; x ++) {
                u32 pos = y * stride / sizeof(u32) + x;
                framebuf[pos] = COLOR_SNAKE;
            }
        }
    }
    
    // Draw food
    u32 a = state->foodY * CELL_SIZE;
    u32 b = state->foodX * CELL_SIZE;
    for (u32 y = a + CELL_BORDER; y < a + CELL_SIZE - CELL_BORDER; y ++) {
        for (u32 x = b + CELL_BORDER; x < b + CELL_SIZE - CELL_BORDER; x ++) {
            u32 pos = y * stride / sizeof(u32) + x;
            framebuf[pos] = COLOR_FOOD;
        }
    }
    
    // Draw score
    drawNumber(framebuf, stride, 10, 10, state->score);

    framebufferEnd(&fb);
}

// Main program entrypoint
int main(int argc, char* argv[])
{
    NWindow* win = nwindowGetDefault();
    srand(RANDOM_SEED);

    Framebuffer fb;
    framebufferCreate(&fb, win, FB_WIDTH, FB_HEIGHT, PIXEL_FORMAT_RGBA_8888, 2);
    framebufferMakeLinear(&fb);

    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    PadState pad;
    padInitializeDefault(&pad);

    struct State state = stateInit();

    // Main loop
    while (appletMainLoop())
    {
        checkInput(&pad, &state);
        if (state.quit) break;
        if (state.dead || state.paused) continue;

        draw(fb, &state);
        update(&state);

        state.frame = (state.frame + 1) % FRAME_RATE;
    }

    framebufferClose(&fb);
    free(state.segments);
    return 0;
}
