#ifndef __ST7789_H
#define __ST7789_H

#include "main.h"
#include <stdint.h>

#define ST7789_WIDTH   240
#define ST7789_HEIGHT  320

#define ST7789_BLACK   0x0000
#define ST7789_WHITE   0xFFFF
#define ST7789_RED     0xF800
#define ST7789_GREEN   0x07E0
#define ST7789_BLUE    0x001F
#define ST7789_YELLOW  0xFFE0
#define ST7789_CYAN    0x07FF
#define ST7789_MAGENTA 0xF81F

void ST7789_Init(void);
void ST7789_FillColor(uint16_t color);
void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *image);
void ST7789_DrawImage4BPP(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                          const uint8_t *packed_pixels, const uint16_t *palette);

#endif
