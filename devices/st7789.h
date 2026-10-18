#ifndef ST7789_H
#define ST7789_H

#include <stdint.h>

#define ST7789_WIDTH  240
#define ST7789_HEIGHT 240

// colors in RGB565: 5 bits red, 6 bits green, 5 bits blue
#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF
#define COLOR_RED   0xF800
#define COLOR_GREEN 0x07E0
#define COLOR_BLUE  0x001F

void st7789_init(void);
void st7789_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void st7789_fill_screen(uint16_t color);
void st7789_draw_pixels(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *pixels);

#endif
