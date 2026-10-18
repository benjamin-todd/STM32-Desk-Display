#include "ui.h"
#include "gfx.h"
#include "st7789.h"

#define COLOR_GRAY   0x8410
#define COLOR_ORANGE 0xFD20
#define COLOR_CYAN   0x07FF

static int32_t  shown_temp;
static uint32_t shown_hum;

// turn 2345 into "23.45" (buf must hold at least 12 chars)
static char *format_hundredths(char *buf, int32_t v) {
    char tmp[12];
    int i = 0;

    if (v < 0) {
        *buf++ = '-';
        v = -v;
    }
    uint32_t whole = (uint32_t)v / 100;
    uint32_t frac  = (uint32_t)v % 100;

    do {                                      // whole part, digits come out backwards
        tmp[i++] = '0' + whole % 10;
        whole /= 10;
    } while (whole > 0);
    while (i > 0) *buf++ = tmp[--i];

    *buf++ = '.';
    *buf++ = '0' + frac / 10;
    *buf++ = '0' + frac % 10;
    *buf = '\0';
    return buf;
}

static void draw(void) {
    char text[16];
    char *end;

    // temperature: label + big value
    gfx_draw_text(20, 20, "TEMPERATURE", COLOR_GRAY, 2);
    end = format_hundredths(text, shown_temp);
    end[0] = GFX_DEGREE;                      // degree sign, then "C"
    end[1] = 'C';
    end[2] = '\0';
    gfx_draw_text(20, 50, text, COLOR_ORANGE, 4);

    // humidity: label + big value
    gfx_draw_text(20, 110, "HUMIDITY", COLOR_GRAY, 2);
    end = format_hundredths(text, (int32_t)shown_hum);
    end[0] = '%';
    end[1] = '\0';
    gfx_draw_text(20, 140, text, COLOR_CYAN, 4);

    // 7-segment demo: the whole degrees as big digits (practice for the Phase 8 clock)
    int32_t whole = shown_temp / 100;
    if (whole < 0) whole = -whole;
    gfx_draw_big_digit(150, 190, (whole / 10) % 10, 20, 4, COLOR_WHITE);
    gfx_draw_big_digit(176, 190, whole % 10,        20, 4, COLOR_WHITE);
}

void ui_show_sensor(int32_t temp_c100, uint32_t hum_pct100) {
    shown_temp = temp_c100;
    shown_hum  = hum_pct100;
    gfx_render(COLOR_BLACK, draw);
}
