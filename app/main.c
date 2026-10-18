#include "stm32f4xx.h"
#include "clock.h"
#include "systick.h"
#include "uart.h"
#include "i2c.h"
#include "bme280.h"
#include "st7789.h"
#include "gfx.h"
#include "ui.h"

#define BME280_ADDR 0x76

static void print_ms(const char *label, uint32_t ms) {
    uart_puts(label);
    uart_put_uint(ms);
    uart_puts(" ms\n");
}

int main(void) {
    clock_init();
    systick_init();
    uart_init();
    i2c_init();
    st7789_init();

    uart_puts(GFX_USE_DMA ? "\nDesk display (DMA on)\n" : "\nDesk display (DMA off)\n");
    if (bme280_init(BME280_ADDR)) {
        uart_puts("BME280 not found\n");
    }

    // benchmark once at startup: full-screen solid fill
    uint32_t start = millis();
    gfx_fill_screen(COLOR_BLUE);
    print_ms("full-screen fill: ", millis() - start);
    delay_ms(500);

    while (1) {
        int32_t  temp = 0;
        uint32_t hum  = 0;
        bme280_read(&temp, &hum);

        start = millis();
        ui_show_sensor(temp, hum);
        print_ms("redraw: ", millis() - start);

        delay_ms(1000);
    }
}
