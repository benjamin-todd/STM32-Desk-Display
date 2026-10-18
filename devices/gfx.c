#include "gfx.h"
#include "st7789.h"
#include "font5x7.h"

static uint16_t strip[ST7789_WIDTH * GFX_STRIP_H];
static int16_t  strip_y;   // screen row where the current strip starts

// the screen wants the high byte first, but this chip stores the low byte first in memory.
// swapping once here means the buffer can be sent straight out (and later, by DMA).
static uint16_t swap_bytes(uint16_t c) {
    return (uint16_t)((c >> 8) | (c << 8));
}

void gfx_render(uint16_t bg, void (*draw)(void)) {
    uint16_t bg_sw = swap_bytes(bg);

    for (strip_y = 0; strip_y < ST7789_HEIGHT; strip_y += GFX_STRIP_H) {
        for (uint32_t i = 0; i < ST7789_WIDTH * GFX_STRIP_H; i++) {
            strip[i] = bg_sw;                 // 1. clear the strip
        }
        draw();                               // 2. draw everything (only this strip's rows land)
        st7789_draw_pixels(0, strip_y, ST7789_WIDTH, GFX_STRIP_H, strip);   // 3. send it
    }
}

void gfx_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    // clip the rectangle to the screen width and to the current strip
    int16_t x0 = x < 0 ? 0 : x;
    int16_t x1 = x + w > ST7789_WIDTH ? ST7789_WIDTH : x + w;
    int16_t y0 = y < strip_y ? strip_y : y;
    int16_t y1 = y + h > strip_y + GFX_STRIP_H ? strip_y + GFX_STRIP_H : y + h;
    if (x0 >= x1 || y0 >= y1) return;         // nothing of it is in this strip

    uint16_t c = swap_bytes(color);
    for (int16_t row = y0; row < y1; row++) {
        uint16_t *p = &strip[(row - strip_y) * ST7789_WIDTH + x0];
        for (int16_t col = x0; col < x1; col++) {
            *p++ = c;
        }
    }
}

void gfx_draw_char(int16_t x, int16_t y, char c, uint16_t color, uint8_t scale) {
    uint8_t code = (uint8_t)c;
    if (code < FONT_FIRST || code > FONT_LAST) code = '?';

    // skip characters that are completely above or below this strip
    if (y >= strip_y + GFX_STRIP_H || y + 7 * scale <= strip_y) return;

    const uint8_t *glyph = font5x7[code - FONT_FIRST];
    for (int col = 0; col < 5; col++) {
        uint8_t bits = glyph[col];            // one column, bit 0 = top pixel
        for (int row = 0; row < 7; row++) {
            if (bits & (1 << row)) {
                gfx_fill_rect(x + col * scale, y + row * scale, scale, scale, color);
            }
        }
    }
}

void gfx_draw_text(int16_t x, int16_t y, const char *s, uint16_t color, uint8_t scale) {
    while (*s) {
        gfx_draw_char(x, y, *s++, color, scale);
        x += 6 * scale;                       // 5 pixels wide + 1 pixel gap
    }
}

static const uint8_t segments[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

// a big digit w pixels wide, (2w - t) tall, with segments t pixels thick
void gfx_draw_big_digit(int16_t x, int16_t y, uint8_t digit, int16_t w, int16_t t, uint16_t color) {
    if (digit > 9) return;
    uint8_t s = segments[digit];
    int16_t mid = y + w - t;                  // top of the middle segment

    if (s & 0x01) gfx_fill_rect(x,         y,             w, t, color);   // a
    if (s & 0x02) gfx_fill_rect(x + w - t, y,             t, w, color);   // b
    if (s & 0x04) gfx_fill_rect(x + w - t, mid,           t, w, color);   // c
    if (s & 0x08) gfx_fill_rect(x,         mid + w - t,   w, t, color);   // d
    if (s & 0x10) gfx_fill_rect(x,         mid,           t, w, color);   // e
    if (s & 0x20) gfx_fill_rect(x,         y,             t, w, color);   // f
    if (s & 0x40) gfx_fill_rect(x,         mid,           w, t, color);   // g
}
