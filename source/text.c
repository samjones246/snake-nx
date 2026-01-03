#include "text.h"
#include <stdio.h>

// Draw horizontal line segment
static void drawHSegment(u32* framebuf, u32 stride, u32 x, u32 y, u32 width) {
    for (u32 i = 0; i < width; i++) {
        for (u32 j = 0; j < NUMBER_SEGMENT_SIZE; j++) {
            u32 px = x + i;
            u32 py = y + j;
            if (px < FB_WIDTH && py < FB_HEIGHT) {
                u32 pos = py * stride / sizeof(u32) + px;
                framebuf[pos] = COLOR_NUMBER;
            }
        }
    }
}

// Draw vertical line segment
static void drawVSegment(u32* framebuf, u32 stride, u32 x, u32 y, u32 height) {
    for (u32 i = 0; i < NUMBER_SEGMENT_SIZE; i++) {
        for (u32 j = 0; j < height; j++) {
            u32 px = x + i;
            u32 py = y + j;
            if (px < FB_WIDTH && py < FB_HEIGHT) {
                u32 pos = py * stride / sizeof(u32) + px;
                framebuf[pos] = COLOR_NUMBER;
            }
        }
    }
}

// Draw 7-segment style digit at (x, y)
// Layout: a (top), b (top-right), c (bottom-right), d (bottom), e (bottom-left), f (top-left), g (middle)
void drawDigit(u32* framebuf, u32 stride, u32 x, u32 y, u8 digit) {
    u32 seg_w = NUMBER_SEGMENT_SIZE * 2;
    u32 seg_h = NUMBER_SEGMENT_SIZE * 2;
    u32 gap = NUMBER_SEGMENT_SIZE;
    
    // 7-segment patterns for digits 0-9
    // bits: a b c d e f g
    u8 patterns[] = {
        0b1111110, // 0: a,b,c,d,e,f
        0b0110000, // 1: b,c
        0b1101101, // 2: a,b,d,e,g
        0b1111001, // 3: a,b,c,d,g
        0b0110011, // 4: b,c,f,g
        0b1011011, // 5: a,c,d,f,g
        0b1011111, // 6: a,c,d,e,f,g
        0b1110000, // 7: a,b,c
        0b1111111, // 8: all segments
        0b1111011  // 9: a,b,c,d,f,g
    };
    
    if (digit > 9) return;
    
    u8 pattern = patterns[digit];
    
    // a: top horizontal
    if (pattern & 0b1000000) drawHSegment(framebuf, stride, x + gap, y, seg_w);

    // b: top-right vertical
    if (pattern & 0b0100000) drawVSegment(framebuf, stride, x + gap + seg_w, y + gap, seg_h);

    // c: bottom-right vertical
    if (pattern & 0b0010000) drawVSegment(framebuf, stride, x + gap + seg_w, y + gap + seg_h + gap, seg_h);

    // d: bottom horizontal
    if (pattern & 0b0001000) drawHSegment(framebuf, stride, x + gap, y + gap + seg_h + gap + seg_h, seg_w);

    // e: bottom-left vertical
    if (pattern & 0b0000100) drawVSegment(framebuf, stride, x, y + gap + seg_h + gap, seg_h);
    
    // f: top-left vertical
    if (pattern & 0b0000010) drawVSegment(framebuf, stride, x, y + gap, seg_h);
    
    // g: middle horizontal
    if (pattern & 0b0000001) drawHSegment(framebuf, stride, x + gap, y + gap + seg_h, seg_w);
}

// Draw 7-segment style number at (x, y)
void drawNumber(u32* framebuf, u32 stride, u32 x, u32 y, u32 number) {
    char str[11];
    snprintf(str, sizeof(str), "%u", number);
    u32 len = 0;
    while (str[len] != '\0') len++;
    
    u32 digit_width = NUMBER_SEGMENT_SIZE * 4 + NUMBER_SEGMENT_SIZE * 2;
    for (u32 i = 0; i < len; i++) {
        drawDigit(framebuf, stride, x + i * (digit_width + NUMBER_SEGMENT_SIZE), y, (str[i] - '0'));
    }
}
