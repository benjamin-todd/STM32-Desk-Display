#ifndef GFX_H
#define GFX_H

#include <stdint.h>

#define GFX_STRIP_H 20   // rows per strip: 240 x 20 x 2 bytes = 9.6 KB of RAM
#define GFX_DEGREE  '\x7F'  // put this in a string to draw a degree sign

// 1 = send strips with DMA while the CPU draws the next one (two strip buffers, 19.2 KB)
// 0 = the Phase 6 way: CPU sends every byte itself (one strip buffer, 9.6 KB)
#define GFX_USE_DMA 1

// draw the whole screen: clears each strip to bg, calls draw(), sends the strip
void gfx_render(uint16_t bg, void (*draw)(void));

// same, but only redraws the strips that cover rows y0 up to (not including) y1.
// much faster when only a small part of the screen changed.
void gfx_render_rows(uint16_t bg, void (*draw)(void), int16_t y0, int16_t y1);

// fill the whole screen with one color as fast as possible (for timing comparisons)
void gfx_fill_screen(uint16_t color);

// drawing functions: only call these from inside your draw() function
void gfx_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void gfx_draw_char(int16_t x, int16_t y, char c, uint16_t color, uint8_t scale);
void gfx_draw_text(int16_t x, int16_t y, const char *s, uint16_t color, uint8_t scale);
void gfx_draw_big_digit(int16_t x, int16_t y, uint8_t digit, int16_t w, int16_t t, uint16_t color);

#endif
