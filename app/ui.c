#include "ui.h"
#include "gfx.h"
#include "st7789.h"

#define COLOR_GRAY   0x8410
#define COLOR_ORANGE 0xFD20
#define COLOR_CYAN   0x07FF

static int32_t  shown_temp;
static uint32_t shown_hum;

// turn 2345 into "23.45" (buf must hold at least 12 chars), returns end of string
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

    // Temperature: label + big value
    gfx_draw_text(20, 20, "TEMPERATURE", COLOR_GRAY, 2);
    end = format_hundredths(text, shown_temp);
    end[0] = GFX_DEGREE;                      // degree sign, then "C"
    end[1] = 'C';
    end[2] = '\0';
    gfx_draw_text(20, 50, text, COLOR_ORANGE, 4);
// Humidity: label + big value
    gfx_draw_text(20, 110, "HUMIDITY", COLOR_GRAY, 2);
    end = format_hundredths(text, (int32_t)shown_hum);
    end[0] = '%';
    end[1] = '\0';
    gfx_draw_text(20, 140, text, COLOR_CYAN, 4);

    gfx_draw_text(63, 225, "press KEY to switch", COLOR_GRAY, 1);
}

void ui_show_sensor(int32_t temp_c100, uint32_t hum_pct100) {
    shown_temp = temp_c100;
    shown_hum  = hum_pct100;
    gfx_render(COLOR_BLACK, draw);
}

// ---------- Clock screen ----------

#define DIGIT_W   44    // big digit width; its height is 2 * 44 - 8 = 80
#define DIGIT_T   8     // segment thickness
#define TIME_Y    30    // HH:MM covers rows 30-109
#define SECONDS_Y 123   // seconds cover rows 123-136 (one strip: 120-139)
#define SENSOR_Y  183   // temperature and humidity cover rows 183-196 (one strip: 180-199)

static uint8_t clk_h, clk_m, clk_s;
static int32_t clk_temp;
static uint32_t clk_hum;

static void draw_clock(void) {
    char text[16];
    char *end;

    // HH:MM as big 7-segment digits
    gfx_draw_big_digit(6,   TIME_Y, clk_h / 10, DIGIT_W, DIGIT_T, COLOR_WHITE);
    gfx_draw_big_digit(58,  TIME_Y, clk_h % 10, DIGIT_W, DIGIT_T, COLOR_WHITE);
    gfx_fill_rect(116, TIME_Y + 20, 8, 8, COLOR_WHITE);   // the colon
    gfx_fill_rect(116, TIME_Y + 52, 8, 8, COLOR_WHITE);
    gfx_draw_big_digit(138, TIME_Y, clk_m / 10, DIGIT_W, DIGIT_T, COLOR_WHITE);
    gfx_draw_big_digit(190, TIME_Y, clk_m % 10, DIGIT_W, DIGIT_T, COLOR_WHITE);

    // Seconds, small, centered under the colon
    text[0] = '0' + clk_s / 10;
    text[1] = '0' + clk_s % 10;
    text[2] = '\0';
    gfx_draw_text(109, SECONDS_Y, text, COLOR_GRAY, 2);

    // Temperature and humidity along the bottom
    end = format_hundredths(text, clk_temp);
    end[0] = GFX_DEGREE;
    end[1] = 'C';
    end[2] = '\0';
    gfx_draw_text(18, SENSOR_Y, text, COLOR_ORANGE, 2);

    end = format_hundredths(text, (int32_t)clk_hum);
    end[0] = '%';
    end[1] = '\0';
    gfx_draw_text(150, SENSOR_Y, text, COLOR_CYAN, 2);

    gfx_draw_text(63, 225, "press KEY to switch", COLOR_GRAY, 1);
}

int ui_show_clock(uint8_t h, uint8_t m, uint8_t s, int32_t temp_c100, uint32_t hum_pct100, int full) {
    int time_changed    = (h != clk_h || m != clk_m);
    int seconds_changed = (s != clk_s);
    int sensor_changed  = (temp_c100 != clk_temp || hum_pct100 != clk_hum);

    clk_h = h;
    clk_m = m;
    clk_s = s;
    clk_temp = temp_c100;
    clk_hum  = hum_pct100;

    if (full) {
        gfx_render(COLOR_BLACK, draw_clock);
        return 2;
    }
    if (!time_changed && !seconds_changed && !sensor_changed) {
        return 0;
    }

    // Partial redraw: only send the strips that hold something new
    if (time_changed)    gfx_render_rows(COLOR_BLACK, draw_clock, TIME_Y, TIME_Y + 2 * DIGIT_W - DIGIT_T);
    if (seconds_changed) gfx_render_rows(COLOR_BLACK, draw_clock, SECONDS_Y, SECONDS_Y + 14);
    if (sensor_changed)  gfx_render_rows(COLOR_BLACK, draw_clock, SENSOR_Y, SENSOR_Y + 14);
    return 1;
}
