#include "stm32f4xx.h"
#include "clock.h"
#include "systick.h"
#include "uart.h"
#include "i2c.h"
#include "bme280.h"
#include "st7789.h"
#include "gfx.h"
#include "ui.h"
#include "rtc.h"
#include "button.h"

#define BME280_ADDR    0x76
#define SENSOR_READ_MS 2000   // read the sensor every 2 seconds

enum { SCREEN_CLOCK, SCREEN_SENSOR };

static void print_ms(const char *label, uint32_t ms) {
    uart_puts(label);
    uart_put_uint(ms);
    uart_puts(" ms\n");
}

static void print_2digits(uint8_t v) {
    uart_putc('0' + v / 10);
    uart_putc('0' + v % 10);
}

static void print_time(uint8_t h, uint8_t m, uint8_t s) {
    print_2digits(h);
    uart_putc(':');
    print_2digits(m);
    uart_putc(':');
    print_2digits(s);
    uart_puts("\n");
}

// "14:30" or "14:30:05" -> numbers. Returns 0 if it worked, -1 if the text isn't a valid time.
static int parse_time(const char *t, uint8_t *h, uint8_t *m, uint8_t *s) {
    for (int i = 0; i < 5; i++) {
        if (i == 2 ? t[i] != ':' : (t[i] < '0' || t[i] > '9')) return -1;
    }
    *h = (t[0] - '0') * 10 + (t[1] - '0');
    *m = (t[3] - '0') * 10 + (t[4] - '0');
    *s = 0;
    if (t[5] == ':' && t[6] >= '0' && t[6] <= '9' && t[7] >= '0' && t[7] <= '9') {
        *s = (t[6] - '0') * 10 + (t[7] - '0');
    }
    if (*h > 23 || *m > 59 || *s > 59) return -1;
    return 0;
}

// collect typed characters, when enter is pressed, try to set the clock from the line
// returns 1 if the time was just changed
static int check_uart_time(void) {
    static char line[12];
    static int  len = 0;

    int c = uart_getc();
    if (c < 0) return 0;

    if (c == '\r' || c == '\n') {
        line[len] = '\0';
        len = 0;
        uart_puts("\n");
        uint8_t h, m, s;
        if (parse_time(line, &h, &m, &s) == 0) {
            rtc_set_time(h, m, s);
            uart_puts("time set to ");
            print_time(h, m, s);
            return 1;
        }
        uart_puts("type the time like 14:30 or 14:30:05, then Enter\n");
        return 0;
    }
    if (len < (int)sizeof(line) - 1) {
        line[len++] = (char)c;
        uart_putc((char)c);                   // echo it so you can see what you type
    }
    return 0;
}

int main(void) {
    clock_init();
    systick_init();
    uart_init();
    i2c_init();
    st7789_init();
    button_init();

    uart_puts(GFX_USE_DMA ? "\nDesk display (DMA on)\n" : "\nDesk display (DMA off)\n");
    if (bme280_init(BME280_ADDR)) {
        uart_puts("BME280 not found\n");
    }

    int rtc = rtc_init();
    if (rtc < 0) {
        uart_puts("RTC crystal didn't start\n");
    } else if (rtc == 1) {
        // fresh start (first flash, or power was cut): use the time the code was compiled.
        // __TIME__ is filled in by the compiler, e.g. "14:30:05"
        uint8_t h, m, s;
        parse_time(__TIME__, &h, &m, &s);
        rtc_set_time(h, m, s);
        uart_puts("RTC started, set to build time\n");
    } else {
        uart_puts("RTC was already running\n");
    }
    uart_puts("Type a time (like 14:30) and press Enter to set the clock\n");

    int      screen = SCREEN_CLOCK;
    int      full = 1;                        // redraw everything on the first pass
    int32_t  temp = 0;
    uint32_t hum  = 0;
    uint32_t last_read = 0;
    int      first_read = 1;

    while (1) {
        if (button_was_pressed()) {
            screen = (screen == SCREEN_CLOCK) ? SCREEN_SENSOR : SCREEN_CLOCK;
            full = 1;
        }
        if (check_uart_time()) {
            full = 1;
        }

        int new_reading = 0;
        if (first_read || millis() - last_read >= SENSOR_READ_MS) {
            bme280_read(&temp, &hum);
            last_read = millis();
            first_read = 0;
            new_reading = 1;
        }

        uint32_t start = millis();
        if (screen == SCREEN_CLOCK) {
            uint8_t h, m, s;
            rtc_get_time(&h, &m, &s);
            int drawn = ui_show_clock(h, m, s, temp, hum, full);
            if (drawn == 2) print_ms("clock full redraw: ", millis() - start);
            if (drawn == 1) print_ms("clock partial redraw: ", millis() - start);
        } else if (full || new_reading) {
            ui_show_sensor(temp, hum);
            print_ms("sensor redraw: ", millis() - start);
        }
        full = 0;
    }
}
