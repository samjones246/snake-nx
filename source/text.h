// Seven-segment number drawing header
#ifndef TEXT_H
#define TEXT_H

#include <switch.h>

#ifndef FB_WIDTH
#define FB_WIDTH 1280
#endif

#ifndef FB_HEIGHT
#define FB_HEIGHT 720
#endif

#define NUMBER_SEGMENT_SIZE 10
#define COLOR_NUMBER 0xFFFFFFFF

void drawDigit(u32* framebuf, u32 stride, u32 x, u32 y, u8 digit);
void drawNumber(u32* framebuf, u32 stride, u32 x, u32 y, u32 number);

#endif // TEXT_H
