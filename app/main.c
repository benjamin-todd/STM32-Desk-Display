#include "stm32f4xx.h"
#include "clock.h"
#include "systick.h"
#include "uart.h"
#include "i2c.h"
#include "bme280.h"
#include "st7789.h"
#include "ui.h"

#define BME280_ADDR 0x76

int main(void) {
    clock_init();
    systick_init();
    uart_init();
    i2c_init();
    st7789_init();

    uart_puts("\nDesk display\n");
    if (bme280_init(BME280_ADDR)) {
        uart_puts("BME280 not found\n");
    }

    while (1) {
        int32_t  temp = 0;
        uint32_t hum  = 0;
        bme280_read(&temp, &hum);

        uint32_t start = millis();
        ui_show_sensor(temp, hum);
        uint32_t took = millis() - start;

        uart_puts("redraw: ");
        uart_put_uint(took);
        uart_puts(" ms\n");

        delay_ms(1000);
    }
}
